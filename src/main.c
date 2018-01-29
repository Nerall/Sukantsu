#include "core/detect.h"
#include "core/hand.h"
#include "core/wall.h"
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

	puts("Pop random wall to hand (leave 14 tiles):");
	while (wall.nb_tiles > 14) {
		histo_index_t randi = random_pop_wall(&wall);
		add_tile_hand(&hand, randi);
	}
	puts("OK");

	puts("Wall histogram:");
	print_histo(&wall.histo);
	puts("OK");

	puts("Hand histogram:");
	print_histo(&hand.histo);
	puts("OK");

	puts("Hand is valid:");
	add_group_hand(&hand, 1, SEQUENCE, 1);
	add_group_hand(&hand, 1, PAIR, 2);
	add_group_hand(&hand, 0, TRIPLET, 3);
	add_group_hand(&hand, 1, QUAD, 7);
	add_group_hand(&hand, 1, TRIPLET, 5);
	print_groups(hand.groups);
	printf("\tisvalid: %d\n", isvalid(&hand));
	puts("OK");

	struct hand hand2;
	init_hand(&hand2);
	int TerminalsHonors[] = {0,8,9,17,18,26,27,28,29,30,31,32,33};
	add_tile_hand(&hand2, 32);
	for (int i = 0; i < 13; ++i)
		add_tile_hand(&hand2, TerminalsHonors[i]);
	print_histo(&hand2.histo);
	printf("\tisvalid: %d\n", isvalid(&hand2));
	puts("OK");
}
