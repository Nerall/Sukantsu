#include "player.h"
#include "../console_io.h"
#include "../debug.h"

void init_player(struct player *player, enum player_type player_type) {
	ASSERT_BACKTRACE(player);

	init_hand(&player->hand);
	player->player_type = player_type;
}

static histo_index_t input_AI(struct player *player, enum action *action) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(action);

	struct hand *player_hand = &player->hand;
	*action = ACTION_DISCARD;

	if (player_hand->tenpai) {
		for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
			if (get_histobit(&player_hand->riichitiles, i - 1)) {
				return i - 1;
			}
		}

		ASSERT_BACKTRACE(0 && "RiichiTiles Histobit is empty");
		return NO_TILE_INDEX;
	}

	for (histo_index_t i = HISTO_INDEX_MAX; i > 0; --i) {
		if (player_hand->histo.cells[i - 1]) {
			return i - 1;
		}
	}

	ASSERT_BACKTRACE(0 && "Hand Histogram is empty");
	return NO_TILE_INDEX;
}

static histo_index_t input_console(struct player *player, enum action *action) {
	return get_input(&player->hand.histo, action);
}

histo_index_t get_player_input(struct player *player, enum action *action) {
	ASSERT_BACKTRACE(player);

	switch (player->player_type) {
		case PLAYER_HUMAN:
			return input_console(player, action);

		case PLAYER_AI:
			return input_AI(player, action);

		case PLAYER_AI_TEST:
			ASSERT_BACKTRACE(0 && "No AI-Test set-up");
			return NO_TILE_INDEX;

		default:
			ASSERT_BACKTRACE(0 && "Player-Type not recognized");
			return NO_TILE_INDEX;
	}
}
