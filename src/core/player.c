#include "player.h"
#include "../console_io.h"
#include "../debug.h"
#include "../AI/detect.h"

void init_player(struct player *player, enum player_type player_type) {
	ASSERT_BACKTRACE(player);

	init_hand(&player->hand);
	player->player_type = player_type;
}

// Ask the player for an action until this one is correct
// Applies the action after that
// Return 1 if the player has won
int player_turn(struct player *player, struct grouplist *grouplist,
                       histo_index_t *index_rem) {
	ASSERT_BACKTRACE(player);

	struct hand *player_hand = &player->hand;

	if (is_valid_hand(player_hand, grouplist))
		return 1;

	for (;;) {
		enum action action;
		histo_index_t index = get_player_input(player, &action);
		*index_rem = index;

		switch (action) {
			case ACTION_DISCARD: {
				remove_tile_hand(player_hand, index);
				set_histobit(&player_hand->furitentiles, index);
				add_discard(&player_hand->discardlist, index);
        if (index != player_hand->last_tile) {
					tilestocall(player_hand, grouplist);
					tenpailist(player_hand, grouplist);
				}
				return 0;
			}

			case ACTION_RIICHI: {
				if (player_hand->riichi != NORIICHI || !player_hand->closed ||
				    !get_histobit(&player_hand->riichitiles, index)) {
					break;
				}

				remove_tile_hand(player_hand, index);
				set_histobit(&player_hand->furitentiles, index);
        add_discard(&player_hand->discardlist, index);
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
				break;
				/*
				// if (!get_histobit(&hand->wintiles, index))
				continue;

				wprintf(L"TSUMO!\n\n");
				makegroups(hand, grouplist);

				print_victory(hand, grouplist);
				continue;
				return 1;
				*/
			}

			case ACTION_KAN: {
				break;
			}

			default:
				ASSERT_BACKTRACE(0 && "Action not recognized");
				break;
		}
	}
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
