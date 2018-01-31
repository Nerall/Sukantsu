#include "core/detect.h"
#include "core/hand.h"
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
	printf("\thistobit  : %lu\n", sizeof(struct histobit));
	printf("\tgroup     : %lu\n", sizeof(struct group));
	printf("\thand      : %lu\n", sizeof(struct hand));
	puts("OK");

	puts("Init wall:");
	struct histogram wall;
	init_histogram(&wall, 4);
	puts("OK");

	puts("Wall histogram:");
	print_histo(&wall);
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
		histo_index_t randi = random_pop_histogram(&wall);
		add_tile_hand(&hand, randi);
	}
	puts("OK");

	puts("Wall histogram:");
	print_histo(&wall);
	puts("OK");

	puts("Hand histogram:");
	print_histo(&hand.histo);
	puts("OK");

	puts("Pop the rest of the wall:");
	while (wall.nb_tiles) {
		histo_index_t randi = random_pop_histogram(&wall);
		add_tile_hand(&hand, randi);
	}
	puts("OK");

	puts("Hand is valid:");
	add_group_hand(&hand, 1, SEQUENCE, 1); // 2 3 4
	add_group_hand(&hand, 1, PAIR, 2);     // 3 3
	add_group_hand(&hand, 0, TRIPLET, 3);  // 4 4 4
	add_group_hand(&hand, 1, QUAD, 7);     // 8 8 8 8
	add_group_hand(&hand, 1, TRIPLET, 5);  // 6 6 6
	print_groups(hand.groups);
	printf("\tisvalid: %d\n", isvalid(&hand));
	puts("OK");

	puts("Check valid:");
	struct hand hand2;
	init_hand(&hand2);
	int TerminalsHonors[] = {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33};
	add_tile_hand(&hand2, 32);
	for (int i = 0; i < 13; ++i)
		add_tile_hand(&hand2, TerminalsHonors[i]);
	print_histo(&hand2.histo);
	printf("\tisvalid: %d\n", isvalid(&hand2));
	puts("OK");

	puts("Test histobit");
	struct histobit histobit;
	init_histobit(&histobit, 0);
	printf("\t");
	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		printf("%u ", get_histobit(&histobit, i));
	}
	printf("\n");

	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		set_histobit(&histobit, i);
	}

	printf("\t");
	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		printf("%u ", get_histobit(&histobit, i));
	}
	printf("\n");

	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		clear_histobit(&histobit, i);
	}

	printf("\t");
	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		printf("%u ", get_histobit(&histobit, i));
	}
	printf("\n");

	puts("OK");
}
