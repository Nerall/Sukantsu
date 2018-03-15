#include "riichi_engine.h"
#include "../AI/detect.h"
#include "../console_io.h"
#include "../debug.h"
#include <wchar.h>

void init_riichi_engine(struct riichi_engine *engine) {
	ASSERT_BACKTRACE(engine);

	engine->nb_games = 0;
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

		engine->phase = PHASE_DRAW;

		// Give one tile to the player
		histo_index_t r = random_pop_histogram(&engine->wall);
		add_tile_hand(player_hand, r);

		// Calculate best discards (hints)
		tilestodiscard(player_hand, player_grouplist);


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
	}
}
