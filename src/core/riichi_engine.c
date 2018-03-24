#include "riichi_engine.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"

#include <stdio.h>
#include <time.h>
#include <wchar.h>

#define TIMEOUT_SEND 5
#define TIMEOUT_RECEIVE 15

void init_riichi_engine(struct riichi_engine *engine, enum player_type t1,
                        enum player_type t2, enum player_type t3,
                        enum player_type t4) {
	ASSERT_BACKTRACE(engine);

	init_player(&engine->players[0], t1);
	init_player(&engine->players[1], t2);
	init_player(&engine->players[2], t3);
	init_player(&engine->players[3], t4);

	engine->nb_games = 0;
}

// Verify if the action is valid and return 1 if it is
static int verify_action(struct riichi_engine *engine, struct player *player,
                         struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(input);

	switch (input->action) {
		case ACTION_DISCARD: {
			if (player->hand.histo.cells[input->tile] == 0)
				return 0;
			return 1;
		}

		case ACTION_RIICHI: {
			if (player->hand.riichi != NORIICHI || !player->hand.closed ||
			    !get_histobit(&player->hand.riichitiles, input->tile))
				return 0;
			return 1;
		}

		case ACTION_KAN: {
			// TODO: Kan action
			return 0;
		}

		case ACTION_TSUMO: {
			return is_valid_hand(&player->hand, &engine->grouplist);
		}

		default:
			ASSERT_BACKTRACE(0 && "Action-Type not recognized");
			return 0;
	}
}

static int apply_action(struct riichi_engine *engine, struct player *player,
                        struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	struct hand *player_hand = &player->hand;
	struct grouplist *grouplist = &engine->grouplist;

	switch (input->action) {
		case ACTION_DISCARD: {
			remove_tile_hand(player_hand, input->tile);
			player_hand->discarded_tiles.cells[input->tile] += 1;
			if (input->tile != player_hand->last_tile) {
				tilestocall(player_hand, grouplist);
				tenpailist(player_hand, grouplist);
			}
			return 0;
		}

		case ACTION_RIICHI: {
			remove_tile_hand(player_hand, input->tile);
			player_hand->discarded_tiles.cells[input->tile] += 1;
			tenpailist(player_hand, grouplist);

			// Will be set at RIICHI next turn
			player_hand->riichi = IPPATSU;

			// Init values that will be no more used later
			init_histobit(&player_hand->riichitiles, 0);
			init_histobit(&player_hand->chiitiles, 0);
			init_histobit(&player_hand->pontiles, 0);
			init_histobit(&player_hand->kantiles, 0);
			return 0;
		}

		case ACTION_TSUMO: {
			return 1;
		}

		case ACTION_KAN: {
			break;
		}

		default:
			ASSERT_BACKTRACE(0 && "Action not recognized");
			break;
	}
	return 0;
}

// Play a riichi game and return the index of the player who has won
// If noone has won, return -1
int play_riichi_game(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	struct net_server *server = &engine->server;

	////////////////
	// INIT PHASE //
	////////////////
	++engine->nb_games;
	engine->phase = PHASE_INIT;

	// Initialize structures
	init_histogram(&engine->wall, 4);
	for (int i = 0; i < NB_PLAYERS; ++i) {
		// player_type has been initialized in init_riichi_engine
		init_hand(&engine->players[i].hand);
	}

	// Give 13 tiles to each player
	for (int p = 0; p < NB_PLAYERS; ++p) {
		for (int i = 0; i < 13; ++i) {
			histo_index_t r = random_pop_histogram(&engine->wall);
			add_tile_hand(&engine->players[p].hand, r);
		}
	}

	// [SERVER] Send tiles to all clients
	for (int p = 0; p < NB_PLAYERS; ++p) {
		if (engine->players[p].player_type != PLAYER_CLIENT)
			continue;

		struct player *player = &engine->players[p];

		sfTcpSocket_setBlocking(server->clients[player->net_id], sfFalse);

		if (send_data_to_client(server, player->net_id, &player->hand.histo,
		                        sizeof(struct histogram), TIMEOUT_SEND)) {
			fprintf(stderr, "[ERROR][SERVER] Error while sending init data to"
			                " player %d\n",
			        p);
		}
	}

	// To initialize the waits
	for (int p = 0; p < NB_PLAYERS; ++p) {
		tenpailist(&engine->players[p].hand, &engine->grouplist);
	}

	// Display PHASE_INIT
	display_riichi(engine, 0);

	// Main loop
	for (int p = 0; engine->wall.nb_tiles > 14; p = (p + 1) % NB_PLAYERS) {
		struct player *player = &engine->players[p];
		int is_client = player->player_type == PLAYER_CLIENT;

		////////////////
		// DRAW PHASE //
		////////////////
		engine->phase = PHASE_DRAW;

		// Give one tile to the player
		histo_index_t randi = random_pop_histogram(&engine->wall);
		add_tile_hand(&player->hand, randi);

		// [SERVER] Send tile to client p
		if (is_client) {
			if (send_data_to_client(server, player->net_id, &randi,
			                        sizeof(histo_index_t), TIMEOUT_SEND)) {
				fprintf(stderr,
				        "[ERROR][SERVER] Error while sending popped tile to"
				        " player %d\n",
				        p);
			}
		}

		// Calculate best discards (hints)
		tilestodiscard(&player->hand, &engine->grouplist);

		if (player->player_type == PLAYER_HOST)
			display_riichi(engine, p);

		// GetInput Phase
		engine->phase = PHASE_GETINPUT;
		int win = 0;
		if (is_valid_hand(&player->hand, &engine->grouplist))
			win = 1;

		struct action_input input;
		if (!win) {
			int done = 0;
			time_t t1 = time(NULL);
			while (time(NULL) - t1 < TIMEOUT_RECEIVE && !done) {
				if (is_client) {
					// TODO: [SERVER] Ask client p to send input

					// [SERVER] Receive input from client p
					if (receive_data_from_client(server, player->net_id, &input,
					                             sizeof(struct action_input),
					                             TIMEOUT_RECEIVE)) {
						fprintf(stderr, "[ERROR][SERVER] Error while receiving"
						                " input from player %d\n",
						        p);
					}
				} else {
					get_player_input(player, &input);
				}

				done = verify_action(engine, player, &input);
			}

			if (!done) {
				input.action = ACTION_DISCARD;
				input.tile = random_pop_histogram(&player->hand.histo);
			}

			win = apply_action(engine, player, &input);
		}

		if (win) {
			/////////////////
			// TSUMO PHASE //
			/////////////////
			engine->phase = PHASE_TSUMO;
			display_riichi(engine, p);

			// TODO: [SERVER] Send victory infos to all clients
			// Use net_packet_update

			return p;
		}

		if (is_valid_index(input.tile)) {
			/////////////////
			// CLAIM PHASE //
			/////////////////
			engine->phase = PHASE_CLAIM;

			// TODO: [SERVER] Send claim infos to all clients
			// Use net_packet_update

			for (int p2 = 0; p2 < NB_PLAYERS; ++p2) {
				if (p == p2)
					continue;

				struct player *other_player = &engine->players[p2];

				if (get_histobit(&other_player->hand.wintiles, input.tile)) {
					// Claim the tile
					add_tile_hand(&other_player->hand, input.tile);

					ASSERT_BACKTRACE(
					    is_valid_hand(&other_player->hand, &engine->grouplist));

					// Player p2 win
					engine->phase = PHASE_TSUMO;
					makegroups(&other_player->hand, &engine->grouplist);
					display_riichi(engine, p2);
					return p2;
				}
			}

			// TODO: [SERVER] Receive claim inputs from all clients
			// Use net_packet_input
		}

		////////////////
		// WAIT PHASE //
		////////////////
		engine->phase = PHASE_WAIT;

		// Calculate winning tiles
		tenpailist(&player->hand, &engine->grouplist);

		if (player->player_type == PLAYER_HOST)
			display_riichi(engine, p);
	}

	return -1;
}
