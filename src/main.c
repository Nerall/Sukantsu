#include "core/detect.h"
#include "core/hand.h"
#include "core/detect.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// DEBUG FUNCTION
// Will print an histogram to stdout
static void print_histo(struct histogram *histo) {
	printf("- 0 1 2 3 4 5 6 7 8\n");
	for (int i = 0; i < HISTO_INDEX_MAX;) {
	  printf("  ");
	  for (int j = 0; j < 9 && i < HISTO_INDEX_MAX; ++i, ++j) {
	    printf("%d ", histo->cells[i]);
	  }
	  printf("\n");
	}
}

// DEBUG FUNCTION
// Will print hand groups to stdout
static void print_groups(struct group *groups) {
	printf("\t-> (hidden, type, tile)\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		printf("\t(%d, %d, %d)\n", groups[i].hidden, groups[i].type,
		       groups[i].tile);
	}
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
	  printf("%u\n", randi);
	  add_tile_hand(&hand, randi);
	  print_histo(&hand.histo);
	  makegroup(&hand);
	  histo_index_t index;
	  scanf("%hhu", &index);
	  remove_tile_hand(&hand, index);

	  random_pop_histogram(&wall);
	  random_pop_histogram(&wall);
	  random_pop_histogram(&wall);
	}
}
