#include "riichi_engine.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"
#include "../network/net_packet.h"

#include <stdio.h>
#include <time.h>
#include <wchar.h>

#define TIMEOUT_SEND 5
#define TIMEOUT_RECEIVE 15

void init_riichi_engine(struct riichi_engine *engine, enum player_type t1,
                        enum player_type t2, enum player_type t3,
                        enum player_type t4) {
	ASSERT_BACKTRACE(engine);

	init_player(&engine->players[0], t1, NORTH);
	init_player(&engine->players[1], t2, EAST);
	init_player(&engine->players[2], t3, SOUTH);
	init_player(&engine->players[3], t4, EAST);

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

static int verify_and_claim(struct riichi_engine *engine, int player_index,
                            struct action_input input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(player_index >= 0 && player_index < NB_PLAYERS);

	struct player *player = &engine->players[player_index];
	struct net_server *server = &engine->server;

	if (get_histobit(&player->hand.wintiles, input.tile)) {
		// Claim the tile
		add_tile_hand(&player->hand, input.tile);

		ASSERT_BACKTRACE(is_valid_hand(&player->hand, &engine->grouplist));

		// Player c win
		engine->phase = PHASE_TSUMO;
		makegroups(&player->hand, &engine->grouplist);
		display_riichi(engine, player_index);

		struct net_packet_update v_packet = {
			packet_type : PACKET_UPDATE,
			player_pos : player->player_pos,
			input : input,
			victory : 1
		};

		for (int c = 0; c < NB_PLAYERS; ++c) {
			if (engine->players[c].player_type != PLAYER_CLIENT)
				continue;

			int net_id = engine->players[c].net_id;

			if (send_data_to_client(server, net_id, &v_packet,
			                        sizeof(struct net_packet_update),
			                        TIMEOUT_SEND)) {
				fprintf(stderr, "[ERROR][SERVER] Error while sending"
				                " victory packet to player %d\n",
				        c);
			}
		}

		return player_index;
	}

	return -1;
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

	// Give 13 tiles to each player
	for (int p = 0; p < NB_PLAYERS; ++p) {
		// player_type has been initialized in init_riichi_engine
		init_hand(&engine->players[p].hand);
		for (int i = 0; i < 13; ++i) {
			histo_index_t r = random_pop_histogram(&engine->wall);
			add_tile_hand(&engine->players[p].hand, r);
		}
	}

	// [SERVER] Send tiles to all clients
	for (int p = 0; p < NB_PLAYERS; ++p) {
		struct player *player = &engine->players[p];

		if (player->player_type != PLAYER_CLIENT)
			continue;

		sfTcpSocket_setBlocking(server->clients[player->net_id], sfFalse);

		struct net_packet_init packet = {
			packet_type : PACKET_INIT,
			histo : player->hand.histo
		};

		if (send_data_to_client(server, player->net_id, &packet,
		                        sizeof(struct net_packet_init), TIMEOUT_SEND)) {
			fprintf(stderr, "[ERROR][SERVER] Error while sending"
			                " init data to player %d\n",
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
			struct net_packet_draw packet = {
				packet_type : PACKET_DRAW,
				tile : randi
			};

			if (send_data_to_client(server, player->net_id, &packet,
			                        sizeof(struct net_packet_draw),
			                        TIMEOUT_SEND)) {
				fprintf(stderr, "[ERROR][SERVER] Error while sending"
				                " popped tile to player %d\n",
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
					struct net_packet_input packet = {
						packet_type : PACKET_INPUT
					};

					// [SERVER] Ask client p to send input
					if (send_data_to_client(server, player->net_id, &packet,
					                        sizeof(struct net_packet_input),
					                        TIMEOUT_SEND)) {
						fprintf(stderr, "[ERROR][SERVER] Error while asking"
						                " input to player %d\n",
						        p);
					}

					// [SERVER] Receive input from client p
					if (receive_data_from_client(
					        server, player->net_id, &packet,
					        sizeof(struct net_packet_input), TIMEOUT_RECEIVE)) {
						fprintf(stderr, "[ERROR][SERVER] Error while"
						                " receiving input from player %d\n",
						        p);
					}

					input = packet.input;
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

			// [SERVER] Send victory infos to all clients
			struct net_packet_update packet = {
				packet_type : PACKET_UPDATE,
				player_pos : player->player_pos,
				input : input,
				victory : 1
			};

			for (int c = 0; c < NB_PLAYERS; ++c) {
				if (engine->players[c].player_type != PLAYER_CLIENT)
					continue;

				int net_id = engine->players[c].net_id;

				if (send_data_to_client(server, net_id, &packet,
				                        sizeof(struct net_packet_update),
				                        TIMEOUT_SEND)) {
					fprintf(stderr, "[ERROR][SERVER] Error while sending"
					                " victory packet to player %d\n",
					        c);
				}
			}

			return p;
		}

		if (is_valid_index(input.tile)) {
			/////////////////
			// CLAIM PHASE //
			/////////////////
			engine->phase = PHASE_CLAIM;

			// [SERVER] Send claim infos to all clients
			struct net_packet_update packet = {
				packet_type : PACKET_UPDATE,
				player_pos : player->player_pos,
				input : input,
				victory : 0
			};

			for (int c = 0; c < NB_PLAYERS; ++c) {
				if (engine->players[c].player_type != PLAYER_CLIENT)
					continue;

				int net_id = engine->players[c].net_id;

				if (send_data_to_client(server, net_id, &packet,
				                        sizeof(struct net_packet_update),
				                        TIMEOUT_SEND)) {
					fprintf(stderr, "[ERROR][SERVER] Error while sending"
					                " update packet to player %d\n",
					        c);
				}
			}

			// [SERVER] Potentially receive claim packets from clients
			time_t t1 = time(NULL);
			int claim_client = -1;
			while (claim_client != -1 && time(NULL) - t1 < TIMEOUT_RECEIVE) {
				for (int c = 0; claim_client != -1 && c < NB_PLAYERS; ++c) {
					struct player *other_player = &engine->players[c];
					if (other_player->player_type != PLAYER_CLIENT)
						continue;

					struct net_packet_input packet;

					if (receive_data_from_client(
					        server, player->net_id, &packet,
					        sizeof(struct net_packet_input), 0)) {
						// If no claim, continue with next player
						continue;
					}

					// Claim received, verify the claim
					int player_claim = verify_and_claim(engine, c, input);
					if (player_claim != -1)
						return c;
				}
			}

			for (int p2 = 0; p2 < NB_PLAYERS; ++p2) {
				if (p == p2)
					continue;

				// Claim the tile, verify the claim
				int player_claim = verify_and_claim(engine, p2, input);
				if (player_claim != -1)
					return p2;
			}
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
