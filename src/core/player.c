#include "player.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"
#include "../network/net_client.h"
#include "../network/net_packets.h"
#include "groups.h"
#include "hand.h"
#include "histogram.h"
#include "player_s.h"
#include "riichi_engine.h"
#include "riichi_engine_s.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

typedef struct net_packet_init pk_init;
typedef struct net_packet_input pk_input;
typedef struct net_packet_draw pk_draw;
typedef struct net_packet_tsumo pk_tsumo;
typedef struct net_packet_update pk_update;

void init_player(struct player *player, enum player_type player_type,
                 enum table_pos player_pos) {
	ASSERT_BACKTRACE(player);

	init_hand(&player->hand);
	player->player_type = player_type;
	player->player_pos = player_pos;
}

static void input_console(const struct player *player,
                          struct action_input *input) {
	ASSERT_BACKTRACE(player);

	input->tile = get_input(&player->hand.histo, &input->action);
}

static void input_AI(struct player *player, struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	struct hand *player_hand = &player->hand;
	input->action = ACTION_DISCARD;
	input->tile = NO_TILE_INDEX;

	// Take "win" tile
	if (player_hand->tenpai) {
		for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
			if (get_histobit(&player_hand->riichitiles, i - 1)) {
				input->tile = i - 1;
				return;
			}
		}

		ASSERT_BACKTRACE(0 && "RiichiTiles Histobit is empty");
		return;
	}

	struct histogram tiles_remaining;
	init_histogram(&tiles_remaining, 4);

	struct histogram histocopy;
	groups_to_histo(player_hand, &histocopy);

	struct grouplist grouplist;
	init_grouplist(&grouplist);

	for (histo_index_t i = 0; i < player_hand->discardlist.nb_discards; ++i) {
		tiles_remaining.cells[player_hand->discardlist.discards[i]] -= 1;
	}

	for (histo_index_t i = 0; i < HISTO_INDEX_MAX; ++i) {
		tiles_remaining.cells[i] -= histocopy.cells[i];
	}

	// Take "best" tile
	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1] == 0)
			continue;

		remove_tile_hand(player_hand, i - 1);
		for (histo_index_t j = 0; j < HISTO_INDEX_MAX; ++j) {
			if (tiles_remaining.cells[j] == 0)
				continue;

			add_tile_hand(player_hand, j);
			tenpailist(player_hand, &grouplist);
			if (player_hand->tenpai) {
				input->tile = i - 1;
				remove_tile_hand(player_hand, j);
				add_tile_hand(player_hand, i - 1);
				return;
			}
			remove_tile_hand(player_hand, j);
		}
		add_tile_hand(player_hand, i - 1);
	}

	// Take last tile
	tenpailist(player_hand, &grouplist);
	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1]) {
			input->tile = i - 1;
			return;
		}
	}

	ASSERT_BACKTRACE(0 && "Hand Histogram is empty");
}

// Work in Progress
void apply_call(struct player *player, const struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input->tile > 0 && input->tile < NO_TILE_INDEX);

	struct hand *hand = &player->hand;
	add_tile_hand(hand, input->tile);
	hand->has_claimed = 1;
	switch (input->action) {
		case ACTION_CHII: {
			histo_index_t index = input->chii_first_tile;
			ASSERT_BACKTRACE(get_histobit(&hand->chiitiles, input->tile));
			ASSERT_BACKTRACE(index > 0 && index < NO_TILE_INDEX);

			// Make sure all tiles are in the same family
			ASSERT_BACKTRACE(index % 9 < 7 && index < 25);

			// Make sure we have the tree tiles
			histo_cell_t *cell1 = &hand->histo.cells[index];
			cell1 = cell1; // To remove warning
			ASSERT_BACKTRACE(*cell1 && *(cell1 + 1) && *(cell1 + 2));

			// Add sequence group
			add_group_hand(hand, 0, SEQUENCE, index);
			break;
		}

		case ACTION_PON: {
			ASSERT_BACKTRACE(get_histobit(&hand->pontiles, input->tile));
			ASSERT_BACKTRACE(hand->histo.cells[input->tile] >= 3);
			// Add triplet group
			add_group_hand(hand, 0, TRIPLET, input->tile);
			break;
		}

		case ACTION_KAN: {
			ASSERT_BACKTRACE(get_histobit(&hand->kantiles, input->tile));
			// Find if already triplet, else add quad group
			if (hand->histo.cells[input->tile] == 4) {
				// If no triplet group
				add_group_hand(hand, 0, QUAD, input->tile);
			} else {
				// Find & modify triplet group to quad
				int triplet_found = 0;
				for (int g = 0; g < hand->nb_groups; ++g) {
					if (hand->groups[g].tile == input->tile) {
						ASSERT_BACKTRACE(hand->groups[g].type == TRIPLET);
						hand->groups[g].type = QUAD;

						// We need to remove the tile because
						// we manually changed the group
						remove_tile_hand(hand, input->tile);
						triplet_found = 1;
						break;
					}
				}

				triplet_found = triplet_found; // To avoid compil warnings
				ASSERT_BACKTRACE(triplet_found);
			}

			break;
		}

		case ACTION_RON: {
			ASSERT_BACKTRACE(get_histobit(&hand->wintiles, input->tile));
			// Do nothing more, function's caller will handle victory
			break;
		}

		default: { ASSERT_BACKTRACE(0 && "Not a call"); }
	}
}

void get_player_input(struct player *player, struct action_input *input) {
	ASSERT_BACKTRACE(player);

	switch (player->player_type) {
		case PLAYER_HOST:
			input_console(player, input);
			return;

		case PLAYER_AI:
			input_AI(player, input);
			return;

		case PLAYER_CLIENT:
			input_console(player, input);
			return;

		default:
			fprintf(stderr, "Player-Type not recognized");
			return;
	}
}

void client_main_loop(struct net_client *client) {
	ASSERT_BACKTRACE(client);

	struct riichi_engine engine;
	struct net_packet receiver;
	struct player *player = NULL;
	int iplayer = 0, nb_games = 0;
	while (receive_from_server(client, &receiver, sizeof(struct net_packet))) {
		switch (receiver.packet_type) {
			case PACKET_INIT: {
				// fprintf(stderr, "Received: pk_init\n");
				pk_init *init = (pk_init *)&receiver;

				enum player_type types[4];
				iplayer = (int)init->player_pos;
				for (int i = 0; i < 4; ++i) {
					if (i != iplayer) {
						types[i] = PLAYER_CLIENT;
					} else {
						types[i] = AI_MODE ? PLAYER_AI : PLAYER_HOST;
					}
				}

				init_riichi_engine(&engine, types[0], types[1], types[2],
				                   types[3]);
				engine.nb_games = ++nb_games;

				player = &engine.players[iplayer];

				player->hand.histo = init->histo;
				player->player_pos = init->player_pos;

				engine.phase = PHASE_INIT;
				display_riichi(&engine, iplayer);
				break;
			}

			case PACKET_DRAW: {
				// fprintf(stderr, "Received: pk_draw\n");
				pk_draw *draw = (pk_draw *)&receiver;
				add_tile_hand(&player->hand, draw->tile);

				engine.wall.nb_tiles = draw->nb_wall_tiles;

				engine.phase = PHASE_DRAW;
				display_riichi(&engine, iplayer);

				// Using GUI
				display(&engine, 0);
				struct gameGUI gameGUI;
				init_gameGUI(&gameGUI);
				break;
			}

			case PACKET_INPUT: {
				// fprintf(stderr, "Received: pk_input\n");
				// makegroups(&player->hand, &engine.grouplist);

				pk_input *input = (pk_input *)&receiver;
				get_player_input(player, &input->input);
				send_to_server(client, input, sizeof(pk_input));

				apply_action(player, &input->input);
				engine.phase = PHASE_GETINPUT;
				display_riichi(&engine, iplayer);
				break;
			}

			case PACKET_TSUMO: {
				// fprintf(stderr, "Received: pk_tsumo\n");
				pk_tsumo *tsumo = (pk_tsumo *)&receiver;

				int itsumo = (int)tsumo->player_pos;
				memcpy(&engine.players[itsumo].hand.histo, &tsumo->histo,
				       sizeof(struct histogram));

				makegroups(&engine.players[itsumo].hand, &engine.grouplist);

				engine.phase = PHASE_TSUMO;
				display_riichi(&engine, itsumo);
				break;
			}

			case PACKET_UPDATE: {
				// fprintf(stderr, "Received: pk_update\n");
				pk_update *update = (pk_update *)&receiver;
				struct hand *update_hand =
				    &engine.players[update->player_pos].hand;

				add_discard(&update_hand->discardlist, update->input.tile);
				update_hand->last_discard = update->input.tile;

				engine.phase = PHASE_GETINPUT;
				display_riichi(&engine, update->player_pos);

				if (update->input.action == ACTION_DISCARD &&
				    update->player_pos != player->player_pos) {
					engine.phase = PHASE_CLAIM;
					display_riichi(&engine, update->player_pos);

					/*
					pk_input input = {
					    packet_type : PACKET_INPUT,
					    input : {
					        action : ACTION_PASS,
					        tile : NO_TILE_INDEX,
					    },
					};

					send_to_server(client, &input, sizeof(pk_input));
					*/
				}
				break;
			}

			default:
				ASSERT_BACKTRACE(0 && "Packet-Type not recognized");
				break;
		}
	}
}
