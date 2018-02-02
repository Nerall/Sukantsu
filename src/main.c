#include "core/detect.h"
#include "core/hand.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

// DEBUG FUNCTION
// Will print an histogram to stdout
static void print_histo(struct histogram *histo) {
  wprintf(L" 0 1 2 3 4 5 6 7 8 - Indexes\n");
	wprintf(L" 🀙 🀚 🀛 🀜 🀝 🀞 🀟 🀠 🀡 - Dots\n");
	for (int i = 0; i < 9; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" - Values\n");

	wprintf(L" 9 10 11 12 13 14 15 16 17 - Indexes\n");
	wprintf(L" 🀐 🀑 🀒 🀓 🀔 🀕 🀖 🀗 🀘 - Bamboos\n");
	for (int i = 9; i < 18; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" - Values\n");

	wprintf(L" 18 19 20 21 22 23 24 25 26 - Indexes\n");
	wprintf(L" 🀇 🀈 🀉 🀊 🀋 🀌 🀍 🀎 🀏 - Cracks\n");
	for (int i = 18; i < 27; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" - Values\n");

	wprintf(L" 27 28 29 30 31 32 33 - Indexes\n");
	wprintf(L" 🀀 🀁 🀂 🀃 🀄 🀅 🀆 - Honor tiles\n");
	for (int i = 27; i < 34; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" - Values\n");
}
// DEBUG FUNCTION
// Will print hand groups to stdout
static void print_groups(struct group *groups) {
	wprintf(L"Group\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		wprintf(L"\t(%d, %d, %d)\n", groups[i].hidden, groups[i].type,
		       groups[i].tile);
	}
}

void clear_stream(FILE *in) {
	int ch;
	clearerr(in);
	do {
		ch = getc(in);
	} while (ch != '\n' && ch != EOF);
	clearerr(in);
}

int main() {
	setlocale (LC_ALL, "");
	srand(time(NULL));
	setbuf(stdout, NULL); // To flush automatically stdout

	wprintf(L"Sizeof structures:");
	wprintf(L"\thistogram : %lu\n", sizeof(struct histogram));
	wprintf(L"\thistobit  : %lu\n", sizeof(struct histobit));
	wprintf(L"\tgroup     : %lu\n", sizeof(struct group));
	wprintf(L"\thand      : %lu\n", sizeof(struct hand));

	// Initialization
	struct histogram wall;
	init_histogram(&wall, 4);
	struct hand hand;
	init_hand(&hand);

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		add_tile_hand(&hand, random_pop_histogram(&wall));
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}

	// Main loop
	struct grouplist grouplist;
	while (wall.nb_tiles > 14) {
		// Give one tile to player
		histo_index_t randi = random_pop_histogram(&wall);
		wprintf(L"Tile drawn: %u\n", randi);
		wprintf(L"Draws remaining: %u\n", (wall.nb_tiles - 14) / 4);
		add_tile_hand(&hand, randi);
		print_histo(&hand.histo);

		// Check valid hand
		if (isvalid(&hand)) {
			wprintf(L"YOU WON \\o/");
			break;
		}

		// Check grouplists
		makegroups(&hand, &grouplist);
		for (int i = 0; i < grouplist.nb_groups; ++i) {
			print_groups(grouplist.groups[i]);
		}

		// Ask for tile discard
		unsigned int index = NO_TILE_INDEX;
		while (!is_valid_index(index) || hand.histo.cells[index] == 0) {
			while (scanf("%u", &index) != 1) {
				clear_stream(stdin);
				fflush(stdout);
			}
		}
		remove_tile_hand(&hand, index);

		// Give one tile to each other player
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}

	wprintf(L"End of the game.\n");
}
