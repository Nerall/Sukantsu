#include "core/wall.h"
#include "core/hand.h"
#include "core/detect.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// DEBUG FUNCTION
// Will print an histogram to stdout
static void print_histo(struct histogram *histo) {
	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		printf("\t");
		for (int j = 0; j < 15 && i < HISTO_INDEX_MAX; ++i, ++j) {
			printf("%d ", histo->cells[i]);
		}
		i--;
		printf("\n");
	}
}

int main() {
	srand(time(NULL));

	puts("Sizeof structures:");
	printf("\thistogram : %lu\n", sizeof(struct histogram));
	printf("\tgroup     : %lu\n", sizeof(struct group));
	printf("\twall      : %lu\n", sizeof(struct wall));
	printf("\thand      : %lu\n", sizeof(struct hand));
	puts("OK");

	puts("Init wall:");
	struct wall wall;
	init_wall(&wall);
	puts("OK");

	puts("Wall histogram:");
	print_histo(&wall.histo);
	puts("OK");

	puts("Init hand:");
	struct hand hand;
	init_hand(&hand);
	puts("OK");

	puts("Hand histogram:");
	print_histo(&hand.histo);
	puts("OK");

	puts("Pop random wall to hand:");
	while (wall.nb_tiles > 14) {
		histo_index_t randi = random_pop_wall(&wall);
		add_tile_hand(&hand, randi);
	}
	puts("OK");

	puts("Hand histogram:");
	print_histo(&hand.histo);
	puts("OK");

	puts("Hand is valid:");
	add_group_hand(&hand, 1, PAIR, 1);
	add_group_hand(&hand, 1, PAIR, 2);
	add_group_hand(&hand, 1, PAIR, 3);
	add_group_hand(&hand, 1, PAIR, 4);
	add_group_hand(&hand, 1, PAIR, 5);
	printf("\tisvalid: %d\n",isvalid(&hand));
	puts("OK");

}
