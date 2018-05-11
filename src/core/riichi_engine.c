#include "riichi_engine.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"
#include "../network/net_packets.h"
#include "../network/net_server.h"
#include "groups.h"
#include "hand.h"
#include "histogram.h"
#include "player.h"
#include "riichi_engine_s.h"
#include <stdio.h>

#define TIMEOUT_SEND 5
#define TIMEOUT_RECEIVE 15

typedef struct net_packet_init pk_init;
typedef struct net_packet_draw pk_draw;
typedef struct net_packet_input pk_input;
typedef struct net_packet_update pk_update;

void send_to_all_clients(struct riichi_engine *engine, void *packet,
                         size_t size, int index_to_ignore) {
	struct net_server *server = &engine->server;
	for (int c = 0; c < NB_PLAYERS; ++c) {
		if (engine->players[c].player_type != PLAYER_CLIENT)
			continue;

		if (c == index_to_ignore)
			continue;

		int net_id = engine->players[c].net_id;
		int s = send_data_to_client(server, net_id, packet, size, TIMEOUT_SEND);
		engine->players[c].net_status = !s;
		if (s) {
			fprintf(stderr,
			        "[ERROR][SERVER] Error while sending packet to player %d\n",
			        c);
		}
	}
}

void init_riichi_engine(struct riichi_engine *engine, enum player_type t1,
                        enum player_type t2, enum player_type t3,
                        enum player_type t4) {
	ASSERT_BACKTRACE(engine);

	init_player(&engine->players[0], t1, NORTH);
	init_player(&engine->players[1], t2, EAST);
	init_player(&engine->players[2], t3, SOUTH);
	init_player(&engine->players[3], t4, WEST);

	engine->nb_games = 0;
	engine->server.listener = NULL;
	for (int i = 0; i < NB_PLAYERS; ++i) {
		engine->server.clients[i] = NULL;
	}
}

// Verify if the action is valid and return 1 if it is
static int verify_action(struct riichi_engine *engine, struct player *player,
                         const struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(input);

	switch (input->action) {
	case ACTION_DISCARD: {
		return player->hand.histo.cells[input->tile] != 0;
	}

	case ACTION_RIICHI: {
		return player->hand.riichi == NORIICHI && player->hand.closed &&
		       get_histobit(&player->hand.riichitiles, input->tile);
	}

	case ACTION_KAN: {
		// TODO: Kan action
		return 0;
	}

	case ACTION_TSUMO: {
		return is_valid_hand(&player->hand, &engine->grouplist);
	}

	case ACTION_PASS: {
		return 1;
	}

	default:
		fprintf(stderr, "Action-Type not recognized\n");
		return 0;
	}
}

// Apply the action and return 1 if the player won
int apply_action(struct riichi_engine *engine, struct player *player,
                 const struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	struct hand *player_hand = &player->hand;
	struct grouplist *grouplist = &engine->grouplist;

	switch (input->action) {
	case ACTION_DISCARD: {
		remove_tile_hand(player_hand, input->tile);
		set_histobit(&player_hand->furitentiles, input->tile);
		add_discard(&player_hand->discardlist, input->tile);
		player->hand.last_discard = input->tile;
		if (input->tile != player_hand->last_tile) {
			tilestocall(player_hand, grouplist);
			tenpailist(player_hand, grouplist);
		}
		return 0;
	}

	case ACTION_RIICHI: {
		remove_tile_hand(player_hand, input->tile);
		set_histobit(&player_hand->furitentiles, input->tile);
		add_discard(&player_hand->discardlist, input->tile);
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

	case ACTION_PASS: {
		return 0;
	}

	default:
		ASSERT_BACKTRACE(0 && "Action not recognized");
		break;
	}
	return 0;
}

// TODO //
int verify_and_claim(struct riichi_engine *engine, int player_index,
                     struct action_input input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(player_index >= 0 && player_index < NB_PLAYERS);

	struct player *player = &engine->players[player_index];

	tenpailist(&player->hand, &engine->grouplist);
	if (get_histobit(&player->hand.wintiles, input.tile)) {
		// Claim the tile
		add_tile_hand(&player->hand, input.tile);

		ASSERT_BACKTRACE(is_valid_hand(&player->hand, &engine->grouplist));

		// Player c win
		engine->phase = PHASE_TSUMO;
		makegroups(&player->hand, &engine->grouplist);
		display_riichi(engine, player_index);

		pk_update v_packet = {
			packet_type : PACKET_UPDATE,
			player_pos : player->player_pos,
			input : input,
			victory : 1
		};

		send_to_all_clients(engine, &v_packet, sizeof(v_packet), -1);

		return player_index;
	}

	return -1;
}

// Init phase of a riichi game
void riichi_init_phase(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	struct net_server *server = &engine->server;

	++engine->nb_games;
	engine->phase = PHASE_INIT;

	// Initialize structures
	init_histogram(&engine->wall, 4);

	// Give 13 tiles to each player
	for (int player_index = 0; player_index < NB_PLAYERS; ++player_index) {
		// player_type has been initialized in init_riichi_engine
		init_hand(&engine->players[player_index].hand);

		for (int i = 0; i < 13; ++i) {
			histo_index_t r = random_pop_histogram(&engine->wall);
			add_tile_hand(&engine->players[player_index].hand, r);
		}
	}

	// [SERVER] Send tiles to all clients
	for (int client_index = 0; client_index < NB_PLAYERS; ++client_index) {
		struct player *player = &engine->players[client_index];

		if (player->player_type != PLAYER_CLIENT)
			continue;

		// Warning: sfTrue !
		sfTcpSocket_setBlocking(server->clients[player->net_id], sfTrue);

		pk_init packet = {
			packet_type : PACKET_INIT,
			player_pos : player->player_pos,
			histo : player->hand.histo
		};

		int s = send_data_to_client(server, player->net_id, &packet,
		                            sizeof(pk_init), TIMEOUT_SEND);
		player->net_status = !s;
		if (s) {
			fprintf(stderr,
			        "[ERROR][SERVER] Error while sending"
			        " init data to player %d\n",
			        client_index);
		}
	}

	// To initialize the waits
	for (int player_index = 0; player_index < NB_PLAYERS; ++player_index) {
		tenpailist(&engine->players[player_index].hand, &engine->grouplist);
	}

	// Display PHASE_INIT
	display_riichi(engine, 0);
}

// Draw phase of a riichi game
void riichi_draw_phase(struct riichi_engine *engine, int player_index) {
	ASSERT_BACKTRACE(engine);

	struct net_server *server = &engine->server;
	struct player *player = &engine->players[player_index];
	engine->phase = PHASE_DRAW;

	// Do not draw if already claimed
	if (!player->hand.has_claimed) {
		// Give one tile to the player
		histo_index_t randi = random_pop_histogram(&engine->wall);
		add_tile_hand(&player->hand, randi);

		// DO NOT DELETE THIS !!!
		player->player_type = player->player_type;
		// It should do nothing, but without it,
		// player->player_type value is not really correct (?!?)
		// By using its value just before, it gives the correct value

		// [SERVER] Send tile to client player_index
		if (player->player_type == PLAYER_CLIENT) {
			pk_draw packet = {
				packet_type : PACKET_DRAW,
				nb_wall_tiles : engine->wall.nb_tiles,
				tile : randi
			};
			int s = send_data_to_client(server, player->net_id, &packet,
			                            sizeof(pk_draw), TIMEOUT_SEND);
			player->net_status = !s;
			if (s) {
				fprintf(stderr,
				        "[ERROR][SERVER] Error while sending"
				        " popped tile to player %d\n",
				        player_index);
			}
		}
	}
	player->hand.has_claimed = 0;

	// Calculate best discards (hints)
	tilestodiscard(&player->hand, &engine->grouplist);

	if (player->player_type == PLAYER_HOST)
		display_riichi(engine, player_index);
}

// Get-Input phase of a riichi game
void riichi_get_input_phase(struct riichi_engine *engine, int player_index,
                            struct action_input *player_input) {
	ASSERT_BACKTRACE(engine);

	engine->phase = PHASE_GETINPUT;
	struct net_server *server = &engine->server;
	struct player *player = &engine->players[player_index];

	for (time_t t1 = time(NULL); time(NULL) - t1 < TIMEOUT_RECEIVE;) {
		if (player->player_type != PLAYER_CLIENT) {
			get_player_input(player, player_input);

			if (verify_action(engine, player, player_input)) {
				ASSERT_BACKTRACE(verify_action(engine, player, player_input));
				return;
			}
		}

		pk_input packet = {packet_type : PACKET_INPUT};

		// [SERVER] Ask client player_index to send input
		int s = send_data_to_client(server, player->net_id, &packet,
		                            sizeof(pk_input), TIMEOUT_SEND);
		player->net_status = !s;
		if (s) {
			fprintf(stderr,
			        "[ERROR][SERVER] Error while asking"
			        " input to player %d\n",
			        player_index);
			break;
		}

		// [SERVER] Receive input from client player_index
		receive_data_from_client(server, player->net_id, &packet,
		                         sizeof(pk_input), TIMEOUT_RECEIVE);
		player->net_status = !s;
		if (s) {
			fprintf(stderr,
			        "[ERROR][SERVER] Error while"
			        " receiving input from player %d\n",
			        player_index);
			break;
		}

		if (verify_action(engine, player, &packet.input)) {
			*player_input = packet.input;
			ASSERT_BACKTRACE(verify_action(engine, player, player_input));
			return;
		}
	}

	player_input->tile = NO_TILE_INDEX;
	player_input->action = ACTION_PASS;
	ASSERT_BACKTRACE(verify_action(engine, player, player_input));
	return;
}

// Tsumo phase of a riichi game
void riichi_tsumo_phase(struct riichi_engine *engine, int player_index,
                        struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(input);

	engine->phase = PHASE_TSUMO;
	display_riichi(engine, player_index);

	struct player *player = &engine->players[player_index];

	// [SERVER] Send victory infos to all clients
	pk_update packet = {
		packet_type : PACKET_UPDATE,
		player_pos : player->player_pos,
		input : *input,
		victory : 1
	};

	send_to_all_clients(engine, &packet, sizeof(pk_update), -1);
}

// Claim phase of a riichi game
// Return the index of the player who won (-1 if nobody)
int riichi_claim_phase(struct riichi_engine *engine, int player_index,
                       struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(input);

	engine->phase = PHASE_CLAIM;

	struct player *player = &engine->players[player_index];
	struct net_server *server = &engine->server;

	// [SERVER] Send claim infos to all clients
	pk_update update_packet = {
		packet_type : PACKET_UPDATE,
		player_pos : player->player_pos,
		input : *input,
		victory : 0
	};

	send_to_all_clients(engine, &update_packet, sizeof(pk_update),
	                    player_index);

	char has_passed[NB_PLAYERS];
	for (int i = 0; i < NB_PLAYERS; ++i) {
		has_passed[i] = engine->players[i].player_type != PLAYER_CLIENT;
		has_passed[i] |= (i != player_index);
	}

	int player_claim = -1;
	struct action_input claim_input;

	// [SERVER] Potentially receive claim packets from clients
	{
		time_t t1 = time(NULL);
		int nb_pass;
		do {
			// Don't really work for now
			break;

			nb_pass = 0;
			for (int iclient = 0; iclient < NB_PLAYERS; ++iclient) {
				struct player *other_player = &engine->players[iclient];
				if (has_passed[iclient] || !other_player->net_status) {
					nb_pass++;
					continue;
				}

				pk_input packet;

				if (receive_data_from_client(server, player->net_id, &packet,
				                             sizeof(pk_input), 0)) {
					// If no claim, continue with next player
					continue;
				}

				if (packet.input.action == ACTION_PASS) {
					has_passed[iclient] = 1;
					continue;
				}

				// Claim received, verify the claim
				player_claim = verify_and_claim(engine, iclient, packet.input);

				if (player_claim != -1) {
					claim_input = packet.input;
					break;
				}
			}
		} while (nb_pass < NB_PLAYERS && player_claim == -1 &&
		         time(NULL) - t1 < TIMEOUT_RECEIVE);
	}

	// If no client claim, check AI claim
	if (player_claim != -1) {
		for (int iother = 0; iother < NB_PLAYERS; ++iother) {
			if (player_index == iother ||
			    engine->players[iother].player_type == PLAYER_CLIENT)
				continue;

			// Claim the tile, verify the claim
			player_claim = verify_and_claim(engine, iother, claim_input);
			if (player_claim != -1) {
				break;
			}
		}
	}

	// Send infos if there is a claim
	if (player_claim != -1) {
		engine->players[player_claim].hand.has_claimed = 1;

		if (is_valid_hand(&player->hand, &engine->grouplist)) {
			riichi_tsumo_phase(engine, player_index, &claim_input);
			return player_index;
		}

		pk_update packet = {
			packet_type : PACKET_UPDATE,
			player_pos : (enum table_pos)player_claim,
			input : claim_input,
			victory : 0
		};

		// [SERVER] Send claim infos to all clients
		send_to_all_clients(engine, &packet, sizeof(pk_update), -1);
	}

	return -1;
}

// Play a riichi game and return the index of the player who has won
// If no one has won, return -1
int play_riichi_game(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	riichi_init_phase(engine);

	// Main loop
	for (int player_index = 0; engine->wall.nb_tiles > 14;
	     player_index = (player_index + 1) % NB_PLAYERS) {
		struct player *player = &engine->players[player_index];

		riichi_draw_phase(engine, player_index);

		int win = is_valid_hand(&player->hand, &engine->grouplist);


    //Using GUI
    if (player_index == 0)
      display(engine, player_index);



		struct action_input player_input;
		if (!win) {
			riichi_get_input_phase(engine, player_index, &player_input);

			if (player_input.action == ACTION_PASS) {
				player_input.action = ACTION_DISCARD;
				player_input.tile = random_pop_histogram(&player->hand.histo);
				player->hand.histo.cells[player_input.tile]++;
			}

			ASSERT_BACKTRACE(verify_action(engine, player, &player_input));

			win = apply_action(engine, player, &player_input);
		}

		if (!AI_MODE) {
			engine->phase = PHASE_GETINPUT;
			display_riichi(engine, player_index);
		}

		if (win) {
			riichi_tsumo_phase(engine, player_index, &player_input);
			return player_index;
		}

		if (is_valid_index(player_input.tile)) {
			riichi_claim_phase(engine, player_index, &player_input);
		}

		engine->phase = PHASE_WAIT;

		// Calculate winning tiles
		tenpailist(&player->hand, &engine->grouplist);

		if (player->player_type == PLAYER_HOST)
			display_riichi(engine, player_index);
	}

	return -1;
}
