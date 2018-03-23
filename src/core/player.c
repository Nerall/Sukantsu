#include "player.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"

void init_player(struct player *player, enum player_type player_type) {
	ASSERT_BACKTRACE(player);

	init_hand(&player->hand);
	player->player_type = player_type;
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
