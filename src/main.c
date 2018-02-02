#include "core/detect.h"
#include "core/hand.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// DEBUG FUNCTION
// Will print an histogram to stdout
static void print_histo(struct histogram *histo) {
	printf("--- 0 1 2 3 4 5 6 7 8 - Indexes\n");
	for (int i = 0; i < HISTO_INDEX_MAX;) {
		printf("%2d  ", i);
		for (int j = 0; j < 9 && i < HISTO_INDEX_MAX; ++i, ++j) {
			printf("%d ", histo->cells[i]);
		}
		printf("\n");
	}
}

// DEBUG FUNCTION
// Will print hand groups to stdout
/*
static void print_groups(struct group *groups) {
    printf("\t-> (hidden, type, tile)\n");
    for (int i = 0; i < HAND_NB_GROUPS; ++i) {
        printf("\t(%d, %d, %d)\n", groups[i].hidden, groups[i].type,
               groups[i].tile);
    }
}
*/

void clear_stream(FILE *in) {
	int ch;
	clearerr(in);
	do {
		ch = getc(in);
	} while (ch != '\n' && ch != EOF);
	clearerr(in);
}

int main() {
	srand(time(NULL));
	setbuf(stdout, NULL); // To flush automatically stdout

	puts("Sizeof structures:");
	printf("\thistogram : %lu\n", sizeof(struct histogram));
	printf("\thistobit  : %lu\n", sizeof(struct histobit));
	printf("\tgroup     : %lu\n", sizeof(struct group));
	printf("\thand      : %lu\n", sizeof(struct hand));

	struct histogram wall;
	init_histogram(&wall, 4);
	struct hand hand;
	init_hand(&hand);
	for (int i = 0; i < 13; ++i) {
		add_tile_hand(&hand, random_pop_histogram(&wall));
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}
	while (wall.nb_tiles > 14) {
		histo_index_t randi = random_pop_histogram(&wall);
		printf("Tile drawn: %u\n", randi);
		printf("Draws remaining: %u\n", (wall.nb_tiles - 14) / 4);
		add_tile_hand(&hand, randi);
		print_histo(&hand.histo);
		// if (isvalid(&hand))
		// puts("YOU WON \\o/"); Doesn't work yet
		makegroup(&hand);
		unsigned int index = NO_TILE_INDEX;
		while (!is_valid_index(index) || hand.histo.cells[index] == 0) {
			while (scanf("%u", &index) != 1) {
				clear_stream(stdin);
				fflush(stdout);
			}
		}
		remove_tile_hand(&hand, index);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}
}
