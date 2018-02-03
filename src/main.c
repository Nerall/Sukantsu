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
  printf("--- 0 1 2 3 4 5 6 7 8 - Indexes\n");
  printf(" |  Dots (p)\n 0 ");
	for (int i = 0; i < 9; ++i)
		printf(" %d", histo->cells[i]);
  printf("\n |  Bamboos (s)\n 9 ");
	for (int i = 9; i < 18; ++i)
		printf(" %d", histo->cells[i]);
	printf("\n |  Cracks (m)\n 18");
	for (int i = 18; i < 27; ++i)
		printf(" %d", histo->cells[i]);
	printf("\n |  Honor tiles (z)\n 27");
	for (int i = 27; i < 34; ++i)
		printf(" %d", histo->cells[i]);
	printf("\n");
}

// DEBUG FUNCTION
// Will print hand groups to stdout
static void print_groups(struct group *groups) {
	printf("Group\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		printf("\t(%d, %d, %d)\n", groups[i].hidden, groups[i].type,
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

  printf("Sizeof structures:");
	printf("\thistogram : %lu\n", sizeof(struct histogram));
	printf("\thistobit  : %lu\n", sizeof(struct histobit));
	printf("\tgroup     : %lu\n", sizeof(struct group));
	printf("\thand      : %lu\n", sizeof(struct hand));

	// Initialization
	struct histogram wall;
	init_histogram(&wall, 4);
	struct hand hand;
	init_hand(&hand);
  struct grouplist grouplist;
  histo_cell_t starthand[] = { 3, 4, 4, 5, 6, 6, 7, 8, 8, 8, 10, 11, 12, 22 };
	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
    add_tile_hand(&hand, starthand[i]);
		//add_tile_hand(&hand, random_pop_histogram(&wall));
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}

	// Main loop
	while (wall.nb_tiles > 14) {
		// Give one tile to player
		histo_index_t randi = random_pop_histogram(&wall);
		printf("\nTile drawn: %u\n", randi);
		printf("Draws remaining: %u\n\n", (wall.nb_tiles - 14) / 4);
		add_tile_hand(&hand, randi);
		print_histo(&hand.histo);
    
    // Check hand tenpai
    tenpailist(&hand, &grouplist);
    if (hand.tenpai) {
      printf("You win if you draw:\n");
      for (int h = 0; h < 34; ++h) {
        if (hand.wintiles.cells[h] >= 1) {
          printf("%u\n", h);
        }
      }
    }

		// Check valid hand
		if (isvalid(&hand, &grouplist)) {
			printf("YOU WON \\o/");
			for (int i = 0; i < grouplist.nb_groups; ++i) {
        print_groups(grouplist.groups[i]);
      }
      return 1;
		}

		// Check grouplists
		//makegroups(&hand, &grouplist);

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

	printf("End of the game.\n");
  return 0;
}
