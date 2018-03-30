#include "player.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"
#include "../network/net_client.h"
#include "../network/net_packets.h"
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

static void input_console(struct player *player, struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	input->tile = get_input(&player->hand.histo, &input->action);
}

static void input_AI(struct player *player, struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	struct hand *player_hand = &player->hand;
	input->action = ACTION_DISCARD;
	input->tile = NO_TILE_INDEX;

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

	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1]) {
			input->tile = i - 1;
			return;
		}
	}

	ASSERT_BACKTRACE(0 && "Hand Histogram is empty");
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
	struct player *player;
	int iplayer, nb_games = 0;
	while (receive_from_server(client, &receiver, sizeof(struct net_packet))) {
		switch (receiver.packet_type) {
			case PACKET_INIT: {
				pk_init *init = (pk_init *)&receiver;

				enum player_type types[4];
				iplayer = (int)init->player_pos;
				for (int i = 0; i < 4; ++i) {
					types[i] = (i == iplayer ? PLAYER_HOST : PLAYER_CLIENT);
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
				pk_draw *draw = (pk_draw *)&receiver;
				add_tile_hand(&player->hand, draw->tile);

				engine.wall.nb_tiles = draw->nb_wall_tiles;

				engine.phase = PHASE_DRAW;
				display_riichi(&engine, iplayer);
				break;
			}

			case PACKET_INPUT: {
				pk_input *input = (pk_input *)&receiver;
				get_player_input(player, &input->input);
				send_to_server(client, input, sizeof(pk_input));

				apply_action(&engine, player, &input->input);
				engine.phase = PHASE_GETINPUT;
				display_riichi(&engine, iplayer);
				break;
			}

			case PACKET_TSUMO: {
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
				pk_update *update = (pk_update *)&receiver;

				engine.players[update->player_pos].hand.last_discard =
				    update->input.tile;

				engine.phase = PHASE_GETINPUT;
				display_riichi(&engine, update->player_pos);

				if (update->input.action == ACTION_DISCARD &&
				    update->player_pos != player->player_pos) {
					engine.phase = PHASE_CLAIM;
					//display_riichi(&engine, update->player_pos);

					/*pk_input input = {
					    packet_type : PACKET_INPUT,
					    input : {
					        action : ACTION_PASS,
					        tile : NO_TILE_INDEX,
					    },
					};*/

					// fprintf(stderr, "sending...\n");
					// send_to_server(client, &input, sizeof(pk_input));
				}
				break;
			}

			default:
				ASSERT_BACKTRACE(0 && "Packet-Type not recognized");
				break;
		}
	}
}
