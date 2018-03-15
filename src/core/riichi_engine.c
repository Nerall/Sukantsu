#include "riichi_engine.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"
#include <wchar.h>

void init_riichi_engine(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	engine->nb_games = 0;
}

// Ask the player for an action until this one is correct
// Applies the action after that
// Return 1 if the player has won
//
// TODO: player_turn needs reworking !!
//
static int player_turn(struct hand *hand, struct grouplist *grouplist) {
	if (isvalid(hand, grouplist))
		return 1;

	while (1) {
		enum action action;
		histo_index_t index = NO_TILE_INDEX;
		if (AI_MODE) {
			int i = 33;
			if (hand->tenpai) {
				while (index == NO_TILE_INDEX && i > -1) {
					if (get_histobit(&hand->riichitiles, i))
						index = i;
					--i;
				}
			}
			i = 33;
			while (index == NO_TILE_INDEX) {
				if (hand->histo.cells[i])
					index = i;
				--i;
			}
			action = ACTION_DISCARD;
		}

		else
			index = get_input(&hand->histo, &action);

		char f, n;
		if (index != NO_TILE_INDEX)
			index_to_char(index, &f, &n);

		if (action == ACTION_DISCARD) {
			wprintf(L"action -> discard (%c%c)\n", n, f);
			remove_tile_hand(hand, index);
			hand->discarded_tiles.cells[index] += 1;
			if (index != hand->last_tile) {
				tilestocall(hand, grouplist);
				tenpailist(hand, grouplist);
			}
			return 0;
		}

		if (action == ACTION_RIICHI) {
			wprintf(L"action -> riichi (%c%c)\n", n, f);
			if (hand->riichi != NORIICHI || !hand->closed ||
			    !get_histobit(&hand->riichitiles, index)) {
				continue;
			}

			remove_tile_hand(hand, index);
			hand->discarded_tiles.cells[index] += 1;
			tenpailist(hand, grouplist);

			// Will be set at RIICHI next turn
			hand->riichi = IPPATSU;

			// Init values that will be no more used later
			init_histobit(&hand->riichitiles, 0);
			init_histobit(&hand->chiitiles, 0);
			init_histobit(&hand->pontiles, 0);
			init_histobit(&hand->kantiles, 0);
			return 0;
		}

		if (action == ACTION_TSUMO) {
			wprintf(L"action -> tsumo\n"); //(%c%c)\n", n, f);
			// if (!get_histobit(&hand->wintiles, index))
			continue;

			wprintf(L"TSUMO!\n\n");
			makegroups(hand, grouplist);

			print_victory(hand, grouplist);
			continue;
			return 1;
		}

		if (action == ACTION_KAN) {
			wprintf(L"action -> kan\n");
			continue;
			return 0;
		}

		ASSERT_BACKTRACE(
		    0 && "Action not recognized (someone did not do his job)\n");
	}
	return 0;
}

void play_riichi_game(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	engine->phase = PHASE_INIT;

	// Initialize structures
	init_histogram(&engine->wall, 4);
	for (int i = 0; i < NB_PLAYERS; ++i) {
		init_hand(&engine->hands[i]);
	}

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		for (int p = 0; p < NB_PLAYERS; ++p) {
			histo_index_t r = random_pop_histogram(&engine->wall);
			add_tile_hand(&engine->hands[p], r);
		}
	}

	// To initialize the waits
	for (int p = 0; p < NB_PLAYERS; ++p) {
		tenpailist(&engine->hands[p], &engine->grouplists[p]);
	}

	// Display PHASE_INIT
	display_riichi(engine, 0);

	// Main loop
	for (int p = 0; engine->wall.nb_tiles > 14; p = (p + 1) % NB_PLAYERS) {
		struct hand *player_hand = &engine->hands[p];
		struct grouplist *player_grouplist = &engine->grouplists[p];

		// Draw Phase
		engine->phase = PHASE_DRAW;

		// Give one tile to the player
		histo_index_t r = random_pop_histogram(&engine->wall);
		add_tile_hand(player_hand, r);

		// Calculate best discards (hints)
		tilestodiscard(player_hand, player_grouplist);

		display_riichi(engine, p);

		// GetInput Phase
		engine->phase = PHASE_GETINPUT;
		int win = player_turn(player_hand, player_grouplist);

		// Tsumo Phase (if win)
		if (win) {
			engine->phase = PHASE_TSUMO;
			display_riichi(engine, p);
			return;
		}

		// Wait Phase
		engine->phase = PHASE_WAIT;

		// Calculate winning tiles
		tenpailist(player_hand, player_grouplist);

		display_riichi(engine, p);
	}
}

// Display informations based on the structure
// Current game phase can be obtained via the enum engine->phase
// Current player should not be needed in the future (but still useful now)
void display_riichi(struct riichi_engine *engine, int current_player) {
	ASSERT_BACKTRACE(engine);

	if (engine->phase == PHASE_INIT) {
		wprintf(L"\nGame %u:\n\n", engine->nb_games);
		return;
	}

	if (engine->phase == PHASE_DRAW) {
		struct hand *player_hand = &engine->hands[current_player];

		wprintf(L"--------------------------------\n\n");
		wprintf(L"Remaining tiles: %u\n\n", (engine->wall.nb_tiles - 14));

		print_histo(&player_hand->histo, player_hand->last_tile);

		// Show best discards (hints)
		if (player_hand->tenpai) {
			wprintf(L"You are tenpai if you discard:\n");
			for (int r = 0; r < 34; ++r) {
				if (get_histobit(&player_hand->riichitiles, r)) {
					char f, n;
					index_to_char(r, &f, &n);
					wprintf(L"%c%c %lc\n", n, f, tileslist[r]);
				}
			}
			wprintf(L"\n");
		}
		return;
	}

	if (engine->phase == PHASE_TSUMO) {
		struct hand *player_hand = &engine->hands[current_player];
		struct grouplist *player_grouplist = &engine->grouplists[current_player];

		wprintf(L"TSUMO!\n");
		print_victory(player_hand, player_grouplist);
		return;
	}

	if (engine->phase == PHASE_WAIT) {
		struct hand *player_hand = &engine->hands[current_player];
		if (player_hand->tenpai) {
			wprintf(L"You win if you get:\n");
			for (int w = 0; w < 34; ++w) {
				if (get_histobit(&player_hand->wintiles, w)) {
					char f, n;
					index_to_char(w, &f, &n);
					wprintf(L"%c%c %lc\n", n, f, tileslist[w]);
				}
			}
			wprintf(L"\n");
		}
	}
}
