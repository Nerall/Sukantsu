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
	init_histogram(&player->tiles_remaining, 4);
	player->player_type = player_type;
	player->player_pos = player_pos;
	player->player_score = 25000;
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

	struct grouplist grouplist;
	init_grouplist(&grouplist);

	histo_index_t last_tile = player_hand->last_tile;
	tenpailist(player_hand, &grouplist);

	struct histobit usefultiles;
	init_histobit(&usefultiles, 0);

	int TerminalsHonors[] = {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33};
	int nb_terminals = 0;
	for (int i = 0; i < 13; ++i) {
		if (player_hand->histo.cells[TerminalsHonors[i]])
			++nb_terminals;
	}
	if (nb_terminals >= 9) {
		for (int i = 0; i < 13; ++i) {
			set_histobit(&usefultiles, TerminalsHonors[i]);
		}
	}

	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		if (player_hand->histo.cells[i])
			continue;

		if (i < 27) {
			int mod9 = i % 9;
			if (mod9 > 0) {
				set_histobit(&usefultiles, i - 1);
				if (mod9 > 1) {
					set_histobit(&usefultiles, i - 2);
				}
			}
			if (mod9 < 8) {
				set_histobit(&usefultiles, i + 1);
				if (mod9 < 7) {
					set_histobit(&usefultiles, i + 2);
				}
			}
		}
		set_histobit(&usefultiles, i);
	}

	struct histogram tiles_remaining = player->tiles_remaining;

	struct histogram histocopy;
	groups_to_histo(player_hand, &histocopy);

	histo_index_t best_discard = NO_TILE_INDEX;
	int max_winning_tiles = 0;
	int current_winning_tiles = 0;

	// Take "win" tile
	if (player_hand->tenpai) {
		for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
			if (get_histobit(&player_hand->riichitiles, i - 1)) {
				remove_tile_hand(player_hand, i - 1);
				current_winning_tiles = 0;
				tenpailist(player_hand, &grouplist);
				for (histo_index_t w = 0; w < HISTO_INDEX_MAX; ++w) {
					if (get_histobit(&player_hand->wintiles, w)) {
						current_winning_tiles += tiles_remaining.cells[w];
					}
				}
				add_tile_hand(player_hand, i - 1);
				player_hand->last_tile = last_tile;
				if (current_winning_tiles > max_winning_tiles) {
					best_discard = i - 1;
					max_winning_tiles = current_winning_tiles;
				}
			}
		}
	}

	if (best_discard != NO_TILE_INDEX) {
		input->tile = best_discard;
		player_hand->last_tile = NO_TILE_INDEX;
		// wprintf(L"P%u tenpai\n", player->player_pos + 1);
		return;
	}

	/* Not that useful
	// Take "last_tile" tile
	if (player_hand->last_tile != NO_TILE_INDEX &&
	    get_histobit(&player_hand->furitentiles, player_hand->last_tile)) {
	    input->tile = player_hand->last_tile;
	    player_hand->last_tile = NO_TILE_INDEX;
	    // wprintf(L"P%u furiten\n", player->player_pos + 1);
	    return;
	} */

	// Take "best" tile
	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1] == 0)
			continue;

		remove_tile_hand(player_hand, i - 1);
		current_winning_tiles = 0;
		for (histo_index_t j = 0; j < HISTO_INDEX_MAX; ++j) {
			if (tiles_remaining.cells[j] == 0)
				continue;

			add_tile_hand(player_hand, j);
			tenpailist(player_hand, &grouplist);
			if (player_hand->tenpai) {
				for (histo_index_t w = 0; w < HISTO_INDEX_MAX; ++w) {
					if (get_histobit(&player_hand->wintiles, w)) {
						current_winning_tiles += tiles_remaining.cells[w];
					}
				}
				if (current_winning_tiles > max_winning_tiles) {
					best_discard = i - 1;
					max_winning_tiles = current_winning_tiles;
				}
			}
			remove_tile_hand(player_hand, j);
		}
		add_tile_hand(player_hand, i - 1);
	}

	if (best_discard != NO_TILE_INDEX) {
		input->tile = best_discard;
		player_hand->last_tile = NO_TILE_INDEX;
		// wprintf(L"P%u 1-shanten\n", player->player_pos + 1);
		return;
	}

	// Take "second best" tile
	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1] == 0)
			continue;

		remove_tile_hand(player_hand, i - 1);
		current_winning_tiles = 0;
		for (histo_index_t k = 0; k < HISTO_INDEX_MAX; ++k) {
			if (get_histobit(&usefultiles, k)) {
				if (tiles_remaining.cells[k] == 0)
					continue;

				tiles_remaining.cells[k] -= 1;
				add_tile_hand(player_hand, k);
				for (histo_index_t j = k; j < HISTO_INDEX_MAX; ++j) {
					if (tiles_remaining.cells[j] == 0)
						continue;

					add_tile_hand(player_hand, j);
					tenpailist(player_hand, &grouplist);
					if (player_hand->tenpai) {
						for (histo_index_t w = 0; w < HISTO_INDEX_MAX; ++w) {
							if (get_histobit(&player_hand->wintiles, w)) {
								current_winning_tiles +=
								    tiles_remaining.cells[w];
							}
						}
						if (current_winning_tiles > max_winning_tiles) {
							best_discard = i - 1;
							max_winning_tiles = current_winning_tiles;
						}
					}
					remove_tile_hand(player_hand, j);
				}
				tiles_remaining.cells[k] += 1;
				remove_tile_hand(player_hand, k);
			}
		}
		add_tile_hand(player_hand, i - 1);
	}

	if (best_discard != NO_TILE_INDEX) {
		input->tile = best_discard;
		player_hand->last_tile = NO_TILE_INDEX;
		// wprintf(L"P%u 2-shanten\n", player->player_pos + 1);
		return;
	}

	// Take "third best" tile
	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1] == 0)
			continue;

		remove_tile_hand(player_hand, i - 1);
		current_winning_tiles = 0;
		for (histo_index_t l = 0; l < HISTO_INDEX_MAX; ++l) {
			if (get_histobit(&usefultiles, l)) {
				if (tiles_remaining.cells[l] == 0)
					continue;

				tiles_remaining.cells[l] -= 1;
				add_tile_hand(player_hand, l);
				for (histo_index_t k = l; k < HISTO_INDEX_MAX; ++k) {
					if (get_histobit(&usefultiles, k)) {
						if (tiles_remaining.cells[k] == 0)
							continue;

						tiles_remaining.cells[k] -= 1;
						add_tile_hand(player_hand, k);
						for (histo_index_t j = k; j < HISTO_INDEX_MAX; ++j) {
							if (tiles_remaining.cells[j] == 0)
								continue;

							add_tile_hand(player_hand, j);
							tenpailist(player_hand, &grouplist);
							if (player_hand->tenpai) {
								for (histo_index_t w = 0; w < HISTO_INDEX_MAX;
								     ++w) {
									if (get_histobit(&player_hand->wintiles,
									                 w)) {
										current_winning_tiles +=
										    tiles_remaining.cells[w];
									}
								}
								if (current_winning_tiles > max_winning_tiles) {
									best_discard = i - 1;
									max_winning_tiles = current_winning_tiles;
								}
							}
							remove_tile_hand(player_hand, j);
						}
						tiles_remaining.cells[k] += 1;
						remove_tile_hand(player_hand, k);
					}
				}
				tiles_remaining.cells[l] += 1;
				remove_tile_hand(player_hand, l);
			}
		}
		add_tile_hand(player_hand, i - 1);
	}

	if (best_discard != NO_TILE_INDEX) {
		input->tile = best_discard;
		player_hand->last_tile = NO_TILE_INDEX;
		// wprintf(L"P%u 3-shanten\n", player->player_pos + 1);
		return;
	}

	// Take last tile
	tenpailist(player_hand, &grouplist);
	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1]) {
			input->tile = i - 1;
			player_hand->last_tile = NO_TILE_INDEX;
			// wprintf(L"P%u stupid\n", player->player_pos + 1);
			return;
		}
	}

	ASSERT_BACKTRACE(0 && "Hand Histogram is empty");
}

// Work in Progress
void apply_call(struct player *player, const struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input->tile >= 0 && input->tile < NO_TILE_INDEX);

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
	hand->last_tile = NO_TILE_INDEX;
}

void get_player_input(struct player *player, struct action_input *input) {
	ASSERT_BACKTRACE(player);

	switch (player->player_type) {
		case PLAYER_HOST:
			input_console(player, input);
			return;

		case PLAYER_AI:
			// wprintf(L"before: %u\n", player->hand.histo.nb_tiles);
			input_AI(player, input);
			// wprintf(L"after: %u\n", player->hand.histo.nb_tiles);
			return;

		case PLAYER_CLIENT:
			input_console(player, input);
			return;

		default:
			fprintf(stderr, "Player-Type not recognized");
			return;
	}
}

static int get_index_from_pos(struct riichi_engine *engine,
                              enum table_pos pos) {
	ASSERT_BACKTRACE(engine);
	ASSERT_BACKTRACE(pos == NORTH || pos == EAST || pos == WEST ||
	                 pos == SOUTH);

	for (int i = 0; i < 4; ++i) {
		if (engine->players[i].player_pos == pos) {
			return i;
		}
	}

	ASSERT_BACKTRACE(0 && "Position not found in players");
}

// Update player->tiles_remaining based on **public** infos
void update_tiles_remaining(struct player *player,
                            struct riichi_engine *engine) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(engine);

	// Reset histogram
	init_histogram(&player->tiles_remaining, 4);

	// Remove tiles in our hand
	for (histo_index_t i = 0; i < HISTO_INDEX_MAX; ++i) {
		ASSERT_BACKTRACE(player->hand.histo.cells[i] <= 4);
		player->tiles_remaining.cells[i] -= player->hand.histo.cells[i];
	}

	// Remove tiles in every player's discard
	for (int p = 0; p < 4; ++p) {
		struct discardlist *list = &engine->players[p].hand.discardlist;
		for (unsigned char n = 0; n < list->nb_discards; ++n) {
			ASSERT_BACKTRACE(player->tiles_remaining.cells[list->discards[n]]);
			player->tiles_remaining.cells[list->discards[n]]--;
		}
	}

	// Remove tiles in not hidden groups
	for (int p = 0; p < 4; ++p) {
		struct hand *hand = &engine->players[p].hand;
		for (int g = 0; g < hand->nb_groups; ++g) {
			if (hand->groups[g].hidden)
				continue;

			histo_index_t tile = hand->groups[g].tile;

			switch (hand->groups[g].type) {
				case PAIR:
					ASSERT_BACKTRACE(player->tiles_remaining.cells[tile] >= 2);
					player->tiles_remaining.cells[tile] -= 2;
					break;

				case TRIPLET:
					ASSERT_BACKTRACE(player->tiles_remaining.cells[tile] >= 3);
					player->tiles_remaining.cells[tile] -= 3;
					break;

				case QUAD:
					ASSERT_BACKTRACE(player->tiles_remaining.cells[tile] == 4);
					player->tiles_remaining.cells[tile] -= 4;
					break;

				case SEQUENCE:
					ASSERT_BACKTRACE(player->tiles_remaining.cells[tile]);
					ASSERT_BACKTRACE(player->tiles_remaining.cells[tile + 1]);
					ASSERT_BACKTRACE(player->tiles_remaining.cells[tile + 2]);
					player->tiles_remaining.cells[tile]--;
					player->tiles_remaining.cells[tile + 1]--;
					player->tiles_remaining.cells[tile + 2]--;
					break;

				default:
					ASSERT_BACKTRACE(0 && "Group type not known");
			}
		}
	}
}

void client_main_loop(struct net_client *client) {
	ASSERT_BACKTRACE(client);

	struct riichi_engine engine;
	struct net_packet receiver;
	struct player *player = NULL;
	int iplayer = 0, nb_games = 0;
	int who_won = -666;

	enum player_type our_type = AI_MODE ? PLAYER_AI : PLAYER_HOST;
	init_riichi_engine(&engine, our_type, PLAYER_AI, PLAYER_AI, PLAYER_AI);

	engine.gameGUI.window = sfRenderWindow_create(
	    engine.gameGUI.mode, "Sukantsu (client)", sfResize | sfClose, NULL);

	while (receive_from_server(client, &receiver, sizeof(struct net_packet))) {
		switch (receiver.packet_type) {
			case PACKET_INIT: {
				// fprintf(stderr, "Received: pk_init\n");
				pk_init *init = (pk_init *)&receiver;

				init_hand(&engine.players[0].hand);
				init_hand(&engine.players[1].hand);
				init_hand(&engine.players[2].hand);
				init_hand(&engine.players[3].hand);

				for (int i = 0; i < 4; ++i) {
					engine.players[i].player_pos = (init->player_pos + i) % 4;
					switch (engine.players[i].player_pos) {
						case EAST:
							engine.players[i].player_score = init->score_east;
							break;

						case SOUTH:
							engine.players[i].player_score = init->score_south;
							break;

						case WEST:
							engine.players[i].player_score = init->score_west;
							break;

						case NORTH:
							engine.players[i].player_score = init->score_north;
							break;
					}
				}

				engine.nb_games = ++nb_games;
				player = &engine.players[iplayer];
				player->hand.histo = init->histo;

				engine.phase = PHASE_INIT;
				display_GUI(&engine);
				display_riichi(&engine, iplayer);

				if (who_won != -666) {
					char *pnames[] = {"Nicolas", "Manuel", "Gabriel", "Thibaut"};
					if (who_won == -1) {
						wprintf(L"Result: Draw\n\n");
					} else {
						wprintf(L"Result: %s has won!\n\n", pnames[who_won]);
					}

					who_won = -1;

					for (int i = 0; i < 4; ++i) {
						wprintf(L"%s has %d points.\n", pnames[i],
						        engine.players[i].player_score);
					}
				}

				if (init->end_game) {
					destroy_gameGUI(&engine.gameGUI);
					sfRenderWindow_destroy(engine.gameGUI.window);
					return;
				}

				break;
			}

			case PACKET_DRAW: {
				// fprintf(stderr, "Received: pk_draw\n");
				pk_draw *draw = (pk_draw *)&receiver;
				add_tile_hand(&player->hand, draw->tile);

				engine.wall.nb_tiles = draw->nb_wall_tiles;

				engine.phase = PHASE_DRAW;
				display_GUI(&engine);
				// display_riichi(&engine, iplayer);
				break;
			}

			case PACKET_INPUT: {
				// fprintf(stderr, "Received: pk_input\n");
				// makegroups(&player->hand, &engine.grouplist);

				pk_input *input = (pk_input *)&receiver;
				update_tiles_remaining(player, &engine);
				get_player_input(player, &input->input);
				send_to_server(client, input, sizeof(pk_input));

				apply_action(player, &input->input);

				engine.phase = PHASE_GETINPUT;
				display_GUI(&engine);
				// display_riichi(&engine, iplayer);
				break;
			}

			case PACKET_TSUMO: {
				// fprintf(stderr, "Received: pk_tsumo\n");
				pk_tsumo *tsumo = (pk_tsumo *)&receiver;

				int index_win = 0;
				for (int i = 0; i < 4; ++i) {
					if (engine.players[i].player_pos == tsumo->player_pos) {
						index_win = i;
						break;
					}
				}

				who_won = index_win;

				memcpy(&engine.players[index_win].hand.histo, &tsumo->histo,
				       sizeof(struct histogram));

				makegroups(&engine.players[index_win].hand, &engine.grouplist);
				break;
			}

			case PACKET_UPDATE: {
				// fprintf(stderr, "Received: pk_update\n");
				pk_update *update = (pk_update *)&receiver;

				int index = get_index_from_pos(&engine, update->player_pos);
				struct hand *update_hand = &engine.players[index].hand;

				add_discard(&update_hand->discardlist, update->input.tile);
				update_hand->last_discard = update->input.tile;

				engine.phase = PHASE_GETINPUT;
				display_GUI(&engine);

				// display_riichi(&engine, update->player_pos);

				if (update->input.action == ACTION_DISCARD &&
				    update->player_pos != player->player_pos) {
					engine.phase = PHASE_CLAIM;
					display_GUI(&engine);
					display_riichi(&engine, update->player_pos);

					// TODO: Ask for claim here

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

	destroy_gameGUI(&engine.gameGUI);
	sfRenderWindow_destroy(engine.gameGUI.window);
}
