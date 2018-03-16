#include "AI/detect.h"
#include "console_io.h"
#include "core/riichi_engine.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

static wchar_t tileslist[] = L"🀙🀚🀛🀜🀝🀞🀟🀠🀡🀐🀑🀒🀓🀔🀕🀖🀗🀘🀇🀈🀉🀊🀋🀌🀍🀎🀏🀀🀁🀂🀃🀆🀅🀄";

static int opponent_discard(struct hand *hand, struct grouplist *grouplist,
                            struct histogram *wall, unsigned char player) {
	if (wall->nb_tiles <= 14)
		return 0;

	char *players[] = {"East", "South", "West", "North"};

	histo_index_t discard = random_pop_histogram(wall);
	char f, n;
	index_to_char(discard, &f, &n);
	wprintf(L"%s's discard: %c%c %lc\n\n", players[player], n, f,
	        tileslist[discard]);

	if (get_histobit(&hand->wintiles, discard)) {
		wprintf(L"RON!\n\n");
		add_tile_hand(hand, discard);
		makegroups(hand, grouplist);
		print_victory(hand, grouplist);
		return 1;
	}

	return 0;
}

// Ask the player for an action until this one is correct
// Applies the action after that
// Return 1 if the player has won
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

		fprintf(stderr, "Action not recognized (someone did not do his job)\n");
	}
	return 0;
}

int play(unsigned int nb_games) {
	// Initialization
	struct histogram wall;
	init_histogram(&wall, 4);
	struct hand hand;
	init_hand(&hand);
	struct grouplist grouplist;

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		// add_tile_hand(&hand, starthand[i]);
		add_tile_hand(&hand, random_pop_histogram(&wall));
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}

	wprintf(L"\nGame %u:\n\n", nb_games);

	// To initialize the waits
	tenpailist(&hand, &grouplist);

	// Main loop
	while (wall.nb_tiles > 14) {
		// Give one tile to players
		histo_index_t randi = random_pop_histogram(&wall);
		add_tile_hand(&hand, randi);

		char family, number;
		index_to_char(randi, &family, &number);

		wprintf(L"--------------------------------\n\n");
		wprintf(L"Remaining tiles: %u\n\n", (wall.nb_tiles - 14));

		print_histo(&hand.histo, hand.last_tile);

		// wprintf(L"Tile drawn: %c%c %lc\n\n", number, family,
		// tileslist[randi]);

		// Show best discards
		tilestodiscard(&hand, &grouplist);
		if (hand.tenpai) {
			wprintf(L"You are tenpai if you discard:\n");
			for (int r = 0; r < 34; ++r) {
				if (get_histobit(&hand.riichitiles, r)) {
					char f, n;
					index_to_char(r, &f, &n);
					wprintf(L"%c%c %lc\n", n, f, tileslist[r]);
				}
			}
			wprintf(L"\n");
		}

		if (player_turn(&hand, &grouplist)) {
			wprintf(L"TSUMO!\n");
			print_victory(&hand, &grouplist);
			// The player has won
			return 1;
		}

		// Show winning tiles
		tenpailist(&hand, &grouplist);
		if (hand.tenpai) {
			wprintf(L"You win if you get:\n");
			for (int w = 0; w < 34; ++w) {
				if (get_histobit(&hand.wintiles, w)) {
					char f, n;
					index_to_char(w, &f, &n);
					wprintf(L"%c%c %lc\n", n, f, tileslist[w]);
				}
			}
			wprintf(L"\n");
		}

		// Give one tile to each other player
		if (opponent_discard(&hand, &grouplist, &wall, 1))
			return 1;
		if (opponent_discard(&hand, &grouplist, &wall, 2))
			return 1;
		if (opponent_discard(&hand, &grouplist, &wall, 3))
			return 1;
	}
	wprintf(L"End of the game.\n");
	return 0;
}

int win_at_first_sight(struct histogram *wall, struct hand *hand,
                       struct grouplist *grouplist) {
	// Initialization
	init_histogram(wall, 4);
	init_hand(hand);

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		// add_tile_hand(&hand, starthand[i]);
		add_tile_hand(hand, random_pop_histogram(wall));
		random_pop_histogram(wall);
		random_pop_histogram(wall);
		random_pop_histogram(wall);
	}

	// To initialize the waits
	tenpailist(hand, grouplist);
	histo_index_t randi = random_pop_histogram(wall);
	add_tile_hand(hand, randi);
	if (isvalid(hand, grouplist)) {
		for (int i = 0; i < 33; ++i) {
			for (histo_cell_t j = hand->histo.cells[i]; j > 0; --j) {
				wprintf(L"%lc ", tileslist[i]);
			}
		}
		for (histo_cell_t j = hand->histo.cells[33]; j > 0; --j) {
			wprintf(L"%lc", tileslist[33]);
		}
		return 1;
	}
	return 0;
}

int main() {
	setlocale(LC_ALL, "");
	srand(time(NULL));

	if (FUN_MODE) {
		struct histogram wall;
		struct hand hand;
		struct grouplist grouplist;
		const int big_number = 1 << 22;
		int nb_win = 0;
		wprintf(L"Testing all seeds up to %lld:\n", big_number);
		for (int i = 0; i < big_number; ++i) {
			srand(i);
			if (win_at_first_sight(&wall, &hand, &grouplist)) {
				++nb_win;
				wprintf(L" (seed=%lld)\n", i);
			}
		}
		wprintf(L"Proba: %g\n", 100.0 * (double)nb_win / big_number);
		return 1;
	}

	struct riichi_engine engine;
	init_riichi_engine(&engine, PLAYER_HUMAN, PLAYER_AI, PLAYER_AI, PLAYER_AI);
	char c = 0;
	do {
		int index_win = play(++engine.nb_games);
		//int index_win = play_riichi_game(&engine);
		if (AI_MODE) {
			if (index_win == 0) continue;
			if (index_win != -1)
				break;
			else
				continue;
		}

		wprintf(L"Do you want to continue (y/n)\n> ");
		fflush(stdout);

		do {
			c = getchar();
			if (c >= 'a')
				c += 'A' - 'a';
		} while (c != 'Y' && c != 'N');
		while (getchar() != '\n')
			;
	} while (c != 'N');

	wprintf(L"\nYou played %d game(s).\n", engine.nb_games);
}
