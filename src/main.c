#include "console_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

static int opponent_discard(struct hand *hand, struct grouplist *grouplist,
                            struct histogram *wall, unsigned char player) {
	if (wall->nb_tiles > 14) {
		char *players[] = {"East", "South", "West", "North"};

		histo_index_t discard = random_pop_histogram(wall);
		char f, n;
		index_to_char(discard, &f, &n);
		wprintf(L"%s's discard: %c%c\n\n", players[player], n, f);

		if (get_histobit(&hand->wintiles, discard)) {
			puts("RON!\n");
			add_tile_hand(hand, discard);
			makegroups(hand, grouplist);
			print_victory(hand, grouplist);
			return 1;
		}
	}
	return 0;
}

// Ask the player for an action until this one is correct
// Applies the action after that
// Return 1 if the player has won
static int player_turn(struct hand *hand, struct grouplist *grouplist) {
	while (1) {
		enum action action;
		histo_index_t index = get_input(&hand->histo, &action);

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
			if (hand->riichi == NORIICHI || !hand->closed ||
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
			wprintf(L"action -> tsumo (%c%c)\n", n, f);
			if (!get_histobit(&hand->wintiles, index))
				continue;

			wprintf(L"TSUMO!\n");
			makegroups(hand, grouplist);
			print_victory(hand, grouplist);
			return 1;
		}

		if (action == ACTION_KAN) {
			wprintf(L"action -> kan\n");
			return 0;
		}

		fprintf(stderr, "Action not recognized (someone did not do his job)\n");
	}
	return 0;
}

int play() {
	// Initialization
	struct histogram wall;
	init_histogram(&wall, 4);
	struct hand hand;
	init_hand(&hand);
	struct grouplist grouplist;
	// Next line is for tests
	// histo_cell_t starthand[] = { 0, 0, 9, 9, 18, 18, 27, 27, 29, 29, 31, 32,
	// 33 };

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		// add_tile_hand(&hand, starthand[i]);
		add_tile_hand(&hand, random_pop_histogram(&wall));
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}

	// To initialize the waits
	tenpailist(&hand, &grouplist);
	// Main loop
	while (wall.nb_tiles > 14) {
		// Give one tile to player
		histo_index_t randi = random_pop_histogram(&wall);
		add_tile_hand(&hand, randi);

		char family, number;
		index_to_char(randi, &family, &number);

		wprintf(L"\n-------------------------------\n\n");
		wprintf(L"Remaining tiles: %u\n", (wall.nb_tiles - 14));
		wprintf(L"Tile drawn: %c%c\n\n", number, family);

		/*
		if (get_histobit(&hand.wintiles, randi)) {
		    puts("TSUMO!\n");
		    makegroups(&hand, &grouplist);
		    print_victory(&hand, &grouplist);
		    return 1;
		}
		*/
		print_histo(&hand.histo);

		// Show best discards
		tilestodiscard(&hand, &grouplist);
		if (hand.tenpai) {
			wprintf(L"You are tenpai if you discard:\n");
			for (int r = 0; r < 34; ++r) {
				if (get_histobit(&hand.riichitiles, r)) {
					char f, n;
					index_to_char(r, &f, &n);
					wprintf(L"%c%c\n", n, f);
				}
			}
			wprintf(L"\n");
		}

		if (player_turn(&hand, &grouplist)) {
			// The player has won
			return 1;
		}

		wprintf(L"\n");
		// Show winning tiles
		tenpailist(&hand, &grouplist);
		if (hand.tenpai) {
			wprintf(L"You win if you get:\n");
			for (int w = 0; w < 34; ++w) {
				if (get_histobit(&hand.wintiles, w)) {
					char f, n;
					index_to_char(w, &f, &n);
					wprintf(L"%c%c\n", n, f);
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

int main() {
	setlocale(LC_ALL, "");
	srand(time(NULL));

	wprintf(L"Sizeof structures:\n");
	wprintf(L"\thistogram : %lu\n", sizeof(struct histogram));
	wprintf(L"\thistobit  : %lu\n", sizeof(struct histobit));
	wprintf(L"\tgroup     : %lu\n", sizeof(struct group));
	wprintf(L"\thand      : %lu\n", sizeof(struct hand));

	char c;
	do {
		play();
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
}
