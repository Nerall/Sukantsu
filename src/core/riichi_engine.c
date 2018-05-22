#define _POSIX_C_SOURCE 199309L
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
#include <SFML/Graphics.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#define TIMEOUT_SEND 5
#define TIMEOUT_RECEIVE sfTime_Zero

typedef struct net_packet_init pk_init;
typedef struct net_packet_draw pk_draw;
typedef struct net_packet_input pk_input;
typedef struct net_packet_tsumo pk_tsumo;
typedef struct net_packet_update pk_update;

static void send_to_all_clients(struct riichi_engine *engine, void *packet,
                                size_t size, int index_to_ignore) {
	struct net_server *server = &engine->server;
	for (int c = 0; c < NB_PLAYERS; ++c) {
		if (engine->players[c].player_type != PLAYER_CLIENT)
			continue;

		if (c == index_to_ignore)
			continue;

		int net_id = engine->players[c].net_id;
		int s = send_data_to_client(server, net_id, packet, size);
		engine->players[c].net_status = s;
		if (!s) {
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

	init_player(&engine->players[0], t1, EAST);
	init_player(&engine->players[1], t2, SOUTH);
	init_player(&engine->players[2], t3, WEST);
	init_player(&engine->players[3], t4, NORTH);
	init_gameGUI(&engine->gameGUI);
	engine->nb_games = 0;
	engine->nb_rounds = 0;
	engine->server.listener = NULL;
	for (int i = 0; i < NB_PLAYERS; ++i) {
		engine->server.clients[i] = NULL;
	}
}

// Verify if the action is valid
// Returns 1 if the action is valid, else 0
static int verify_action(struct riichi_engine *engine, struct player *player,
                         const struct action_input *input) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(input);

	// Kinda inefficient, but at least, we're sure
	// Try to change that later if possible
	set_hand_histobits(&player->hand, &engine->grouplist);

	switch (input->action) {
		case ACTION_DISCARD: {
			return player->hand.histo.cells[input->tile] != 0;
		}

		case ACTION_RIICHI: {
			return player->hand.riichi == NORIICHI && player->hand.closed &&
			       get_histobit(&player->hand.riichitiles, input->tile);
		}

		case ACTION_TSUMO: {
			return is_valid_hand(&player->hand, &engine->grouplist);
		}

		case ACTION_CHII: {
			return input->chii_first_tile > 0 &&
			       input->chii_first_tile % 9 < 7 &&
			       input->chii_first_tile < 25 &&
			       get_histobit(&player->hand.chiitiles, input->tile);
		}

		case ACTION_PON: {
			return get_histobit(&player->hand.pontiles, input->tile);
		}

		case ACTION_KAN: {
			return get_histobit(&player->hand.kantiles, input->tile);
		}

		case ACTION_RON: {
			return get_histobit(&player->hand.wintiles, input->tile);
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
// Suppose that the action is allowed
int apply_action(struct player *player, const struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	struct hand *player_hand = &player->hand;

	switch (input->action) {
		case ACTION_DISCARD: {
			remove_tile_hand(player_hand, input->tile);
			set_histobit(&player_hand->furitentiles, input->tile);
			add_discard(&player_hand->discardlist, input->tile);
			player->hand.last_discard = input->tile;
			/*
			if (input->tile != player_hand->last_tile) {
			    tilestocall(player_hand, grouplist);
			    tenpailist(player_hand, grouplist);
			}
			*/
			return 0;
		}

		case ACTION_RIICHI: {
			remove_tile_hand(player_hand, input->tile);
			set_histobit(&player_hand->furitentiles, input->tile);
			add_discard(&player_hand->discardlist, input->tile);
			// tenpailist(player_hand, grouplist);

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
			return input->action == ACTION_TSUMO;
		}

		case ACTION_CHII:
		case ACTION_PON:
		case ACTION_KAN:
		case ACTION_RON: {
			apply_call(player, input);
			return input->action == ACTION_RON;
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
		engine->players[player_index].hand.last_tile = NO_TILE_INDEX;
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
		                            sizeof(pk_init));
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

		/*char *pos[] = {"EAST", "SOUTH", "WEST", "NORTH"};
		char f, n;
		index_to_char(randi, &f, &n);
		wprintf(L"Tile drawn by %s: %c%c\n", pos[player_index], n, f);
		*/

		// [SERVER] Send tile to client player_index
		if (player->player_type == PLAYER_CLIENT) {
			pk_draw packet = {
				packet_type : PACKET_DRAW,
				nb_wall_tiles : engine->wall.nb_tiles,
				tile : randi
			};
			int s = send_data_to_client(server, player->net_id, &packet,
			                            sizeof(pk_draw));
			player->net_status = s;
			if (!s) {
				fprintf(stderr,
				        "[ERROR][SERVER] Error while sending"
				        " popped tile to player %d\n",
				        player_index);
			}
		}
	}
	player->hand.has_claimed = 0;

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

	player_input->tile = NO_TILE_INDEX;
	player_input->action = ACTION_PASS;

	if (player->player_type != PLAYER_CLIENT) {
		update_tiles_remaining(player, engine);
		get_player_input(player, player_input);
	} else {
		pk_input packet = {packet_type : PACKET_INPUT};

		// [SERVER] Ask client player_index to send input
		int s = send_data_to_client(server, player->net_id, &packet,
		                            sizeof(pk_input));
		player->net_status = s;
		if (!s) {
			fprintf(stderr,
			        "[ERROR][SERVER] Error while asking"
			        " input to player %d\n",
			        player_index);
		} else {
			// [SERVER] Receive input from client player_index
			receive_data_from_client(server, player->net_id, &packet,
			                         sizeof(pk_input), TIMEOUT_RECEIVE);
			player->net_status = s;
			if (!s) {
				fprintf(stderr,
				        "[ERROR][SERVER] Error while"
				        " receiving input from player %d\n",
				        player_index);
			}

			*player_input = packet.input;
		}
	}

	if (verify_action(engine, player, player_input)) {
		ASSERT_BACKTRACE(verify_action(engine, player, player_input));
		return;
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
	pk_tsumo packet = {
		packet_type : PACKET_TSUMO,
	};

	memcpy(&packet.histo, &player->hand.histo, sizeof(struct histogram));

	send_to_all_clients(engine, &packet, sizeof(pk_update), -1);
}

static int is_claim_action(enum action action) {
	return action == ACTION_CHII || action == ACTION_PON ||
	       action == ACTION_KAN || action == ACTION_RON;
}

static int is_at_left_of(enum table_pos self, enum table_pos other) {
	// enum = { EAST, SOUTH, WEST, NORTH }
	// NORTH -> EAST -> SOUTH -> WEST -> NORTH
	return (self == NORTH && other == EAST) || (self + 1) == other;
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
	};

	send_to_all_clients(engine, &update_packet, sizeof(pk_update),
	                    player_index);

	struct action_input claim_input = {tile : input->tile};
	int player_claim = -1;

	// [SERVER] Potentially receive claim packets from clients
	for (int c = 0; c < NB_PLAYERS; ++c) {
		// Player who has played can't claim its own tile
		if (c == player_index)
			continue;

		// AI are handled after
		struct player *other_player = &engine->players[c];
		if (other_player->player_type != PLAYER_AI)
			continue;

		if (other_player->player_type == PLAYER_CLIENT) {
			// Here, we treat clients

			pk_input packet;
			// Continue with next player if no claim
			if (!receive_data_from_client(server, player->net_id, &packet,
			                              sizeof(pk_input), TIMEOUT_RECEIVE))
				continue;

			// Pass if player don't want to claim
			if (packet.input.action == ACTION_PASS)
				continue;

			// Verify if the tile is still the same
			if (packet.input.tile != input->tile)
				continue;

			// Verify the action is a claim
			if (!is_claim_action(packet.input.action))
				continue;

			claim_input.action = packet.input.action;
		} else {
			// Here, we treat the host

			// TODO: Ask the host for claim_input
			continue;
		}

		// Chii can only be called to the person at our left
		if (claim_input.action == ACTION_CHII &&
		    !is_at_left_of(other_player->player_pos, player->player_pos))
			continue;

		// Verify the action is valid
		if (!verify_action(engine, other_player, &claim_input))
			continue;

		// Apply the claim
		apply_action(other_player, &claim_input);

		// Remove tile from player's discard list
		pop_last_discard(&player->hand.discardlist);

		player_claim = c;
		break;
	}

	// If no human claimed, check AI claim
	if (player_claim == -1) {
		for (int iother = 0; iother < NB_PLAYERS; ++iother) {
			// Player who has played can't claim its own tile
			if (player_index == iother)
				continue;

			// Only AI are claiming here
			struct player *other_player = &engine->players[iother];
			if (other_player->player_type != PLAYER_AI)
				continue;

			// Verify all claims to see if any is possible
			enum action claims[4] = {ACTION_CHII, ACTION_PON, ACTION_KAN,
			                         ACTION_RON};

			// Modified to forbid claims
			for (int iclaim = 4; iclaim < 4; ++iclaim) {
				claim_input.action = claims[iclaim];
				// Verify the claim
				if (!verify_action(engine, other_player, &claim_input)) {
					continue;
				}

				// Apply the claim
				apply_action(other_player, &claim_input);

				// if (claim_input.action == ACTION_RON)
				// return other_player->player_pos;

				// Remove tile from player's discard list
				pop_last_discard(&player->hand.discardlist);

				player_claim = iother;
				break;
			}

			// Pass others AI if we claimed
			if (player_claim != -1)
				break;
		}
	}

	// Send infos if there is a claim
	if (player_claim != -1) {
		engine->players[player_claim].hand.has_claimed = 1;

		char *str;
		switch (claim_input.action) {
			case ACTION_CHII:
				str = "CHII";
				break;
			case ACTION_PON:
				str = "PON";
				break;
			case ACTION_KAN:
				str = "KAN";
				break;
			case ACTION_RON:
				str = "RON";
				break;
			default:
				str = "ERR";
				break;
		}

		char *pos[] = {"EAST", "SOUTH", "WEST", "NORTH"};
		char f, n;
		index_to_char(input->tile, &f, &n);

		fprintf(stderr, "%s claim of %c%c from player %s!\n", str, n, f,
		        pos[player_claim]);

		if (is_valid_hand(&player->hand, &engine->grouplist)) {
			riichi_tsumo_phase(engine, player_index, &claim_input);
			return player_index;
		}

		pk_update packet = {
			packet_type : PACKET_UPDATE,
			player_pos : (enum table_pos)player_claim,
			input : claim_input,
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

		// Hand has changed => set histobits
		set_hand_histobits(&player->hand, &engine->grouplist);

		int win = is_valid_hand(&player->hand, &engine->grouplist);

		struct action_input player_input;

		// Using GUI
		display_GUI(engine);

		// Sleep for 0.5s
		// const struct timespec delay = {tv_sec : 0, tv_nsec : 500 * 1000000};
		// nanosleep(&delay, NULL);

		if (!win) {
			if (player->hand.riichi != NORIICHI) {
				// A player can't play when he declared riichi
				// So we discard its drawn tile
				player_input.action = ACTION_DISCARD;
				player_input.tile = player->hand.last_tile;
				ASSERT_BACKTRACE(verify_action(engine, player, &player_input));
				apply_action(player, &player_input);
				continue;
			}

			riichi_get_input_phase(engine, player_index, &player_input);

			if (player_input.action == ACTION_PASS) {
				player_input.action = ACTION_DISCARD;
				// We're not modifying hand here (put it back the line after)
				player_input.tile = random_pop_histogram(&player->hand.histo);
				add_tile_hand(&player->hand, player_input.tile);
				player->hand.last_tile = NO_TILE_INDEX;
				// player->hand.histo.cells[player_input.tile]++;
			}

			ASSERT_BACKTRACE(verify_action(engine, player, &player_input));

			win = apply_action(player, &player_input);

			// Hand *may* have changed => set histobits
			set_hand_histobits(&player->hand, &engine->grouplist);
		}

		if (!AI_MODE) {
			engine->phase = PHASE_GETINPUT;
			display_riichi(engine, player_index);
		}

		if (win) {
			riichi_tsumo_phase(engine, player_index, &player_input);
			player->player_won = TSUMO;
			if (engine->players[player_index].player_pos == EAST) {
				engine->players[player_index].player_score += 3000;
				if (engine->players[player_index].player_won == TSUMO) {
					for (int i = 0; i < 4; i++) {
						if (i == player_index)
							continue;
						else
							engine->players[i].player_score -= 1000;
					}
				}
			} else {
				engine->players[player_index].player_score += 2000;
				if (engine->players[player_index].player_won == TSUMO) {
					for (int i = 0; i < 4; i++) {
						if (i == player_index)
							continue;
						if (engine->players[i].player_pos == EAST)
							engine->players[i].player_score -= 1000;
						else
							engine->players[i].player_score -= 500;
					}
				}
			}

			if (engine->players[player_index].player_pos != EAST)
				// if (engine->nb_rounds % NB_PLAYERS == player_index)
				++engine->nb_rounds;

			return engine->players[player_index].player_pos;
		}

		if (is_valid_index(player_input.tile)) {
			riichi_claim_phase(engine, player_index, &player_input);
		}

		engine->phase = PHASE_WAIT;

		// Hand has changed => set histobits
		set_hand_histobits(&player->hand, &engine->grouplist);

		if (player->player_type == PLAYER_HOST)
			display_riichi(engine, player_index);
	}
	if (!engine->players[0].hand.tenpai)
		++engine->nb_rounds;

	int nb_tenpai = 0;
	for (int i = 0; i < 4; i++) {
		if (engine->players[i].hand.tenpai)
			++nb_tenpai;
	}

	if (nb_tenpai == 0 || nb_tenpai == 4)
		return -1;

	for (int i = 0; i < 4; i++) {
		if (engine->players[i].hand.tenpai)
			engine->players[i].player_score += 3000 / nb_tenpai;
		else
			engine->players[i].player_score -= 3000 / (4 - nb_tenpai);
	}

	return -1;
}
