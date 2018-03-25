#include "player.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"

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

void client_main_loop(struct player *player){
	struct net_packet *receiver = malloc(sizeof(struct net_packet));
	if (receive_from_server(player->client, receiver) == 0) 
		return;

	if (receiver->packet_type == PACKET_INIT) {
		struct net_packet_init *init = (struct net_packet_init*) receiver;
		player->hand.histo = init->histo;
		return;
	}
	
	if (receiver->packet_type == PACKET_DRAW) {
		struct net_packet_draw *draw = (struct net_packet_draw*) receiver;
		add_tile_hand(&player->hand, draw->tile);
		return;
	}
	
	if (receiver->packet_type == PACKET_INPUT)
	{
		struct net_packet_input *input = (struct net_packet_input*) receiver;
		struct action_input *action;
		action = NULL;
		get_player_input(player, action);
		input->input = *action;
		if (send_to_server(player->client, input, sizeof(input)) == 0)
			return;
		return;
	}

	if (receiver->packet_type == PACKET_UPDATE)
	{
		struct net_packet_update *update = (struct net_packet_update*) receiver;
		if (update->victory)
		{
			wprintf(L"%s%s%s\n","Player", update->player_pos, "won the game!");
			return;
		}
		else
		{
			if (update->input.action == ACTION_DISCARD)
			{
				wprintf(L"%s%s%s%c\n%s\n", "Player", update->player_pos, "just discarded tile", update->input.tile, "Do you claim?");
				struct action_input claim = {
					action : ACTION_PASS,
					tile : NO_TILE_INDEX,
				};
				struct net_packet_input input = {
					packet_type : PACKET_INPUT,
					input : claim,
				};
				if (send_to_server(player->client, &input, sizeof(&input)) == 0)
					return;
				return;

			}

		}
	}
}