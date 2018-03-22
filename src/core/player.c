#include "player.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"

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
		struct action_input input;
		get_player_input(player, &input);
		*index_rem = input.tile;

		switch (input.action) {
			case ACTION_DISCARD: {
				remove_tile_hand(player_hand, input.tile);
				player_hand->discarded_tiles.cells[input.tile] += 1;
				if (input.tile != player_hand->last_tile) {
					tilestocall(player_hand, grouplist);
					tenpailist(player_hand, grouplist);
				}
				return 0;
			}

			case ACTION_RIICHI: {
				if (player_hand->riichi != NORIICHI || !player_hand->closed ||
				    !get_histobit(&player_hand->riichitiles, input.tile)) {
					break;
				}

				remove_tile_hand(player_hand, input.tile);
				player_hand->discarded_tiles.cells[input.tile] += 1;
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
				// if (!get_histobit(&hand->wintiles, input.tile))
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

static void input_client(struct player *player, struct action_input *input) {
	ASSERT_BACKTRACE(player);
	ASSERT_BACKTRACE(input);

	player = player;
	input = input;
	ASSERT_BACKTRACE(0 && "TODO: Input for network player");
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
			input_client(player, input);
			return;

		default:
			ASSERT_BACKTRACE(0 && "Player-Type not recognized");
			return;
	}
}
