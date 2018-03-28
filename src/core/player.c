#include "player.h"
#include "../console_io.h"
#include "../debug.h"
#include "../network/net_client.h"
#include "../network/net_packets.h"
#include "hand.h"
#include "histogram.h"
#include "player_s.h"
#include <wchar.h>

typedef struct net_packet_init pk_init;
typedef struct net_packet_input pk_input;
typedef struct net_packet_draw pk_draw;
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

		default:
			ASSERT_BACKTRACE(0 && "Player-Type not recognized");
			return;
	}
}

void client_main_loop(struct player *player) {
	ASSERT_BACKTRACE(player);

	struct net_packet receiver;
	if (!receive_from_server(&player->client, &receiver,
	                         sizeof(struct net_packet)))
		return;

	switch (receiver.packet_type) {
		case PACKET_INIT: {
			pk_init *init = (pk_init *)&receiver;
			player->hand.histo = init->histo;
			break;
		}

		case PACKET_DRAW: {
			pk_draw *draw = (pk_draw *)&receiver;
			add_tile_hand(&player->hand, draw->tile);
			break;
		}

		case PACKET_INPUT: {
			pk_input *input = (pk_input *)&receiver;
			get_player_input(player, &input->input);
			send_to_server(&player->client, input, sizeof(pk_input));
			break;
		}

		case PACKET_UPDATE: {
			pk_update *update = (pk_update *)&receiver;
			if (update->victory) {
				wprintf(L"Player %s won the game!\n", update->player_pos);
				break;
			}

			if (update->input.action == ACTION_DISCARD) {
				wprintf(L"Player %s just discarded tile %c\nDo you claim?\n",
				        update->player_pos, update->input.tile);

				pk_input input = {
					packet_type : PACKET_INPUT,
					input : {
						action : ACTION_PASS,
						tile : NO_TILE_INDEX,
					},
				};

				send_to_server(&player->client, &input, sizeof(pk_input));
			}
			break;
		}

		default:
			ASSERT_BACKTRACE(0 && "Packet-Type not recognized");
			break;
	}
}
