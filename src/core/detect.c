#include "detect.h"
#include "../debug.h"
#include <stdio.h>

int isclassical(struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	if (hand->nb_groups < 5)
		return 0;

	int pair = 0;
	for (int i = 0; i < 5; ++i) {
		if (hand->groups[i].type == PAIR) {
			if (pair)
				return 0;
			pair = 1;
		}
	}
	return 1;
}

int ischiitoi(struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	for (int i = 0; i < 34; ++i) {
		if (hand->histo.cells[i] != 0 && hand->histo.cells[i] != 2)
			return 0;
	}
	return 1;
}

int iskokushi(struct hand *hand) {
	ASSERT_BACKTRACE(hand);
	int TerminalsHonors[] = {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33};
	unsigned char pair = 0;
	for (int i = 0; i < 13; ++i)
		if (!pair && hand->histo.cells[TerminalsHonors[i]] == 2)
			pair = 1;
		else if (hand->histo.cells[TerminalsHonors[i]] != 1)
			return 0;
	return 1;
}

// Check if a hand is valid
int isvalid(struct hand *hand) {

	return isclassical(hand) || ischiitoi(hand) || iskokushi(hand);
}

// Recursive function of makegroup
void makegroup_rec(struct hand *hand, int index, struct groupslist *groupslist,
                   unsigned char pair) {
	ASSERT_BACKTRACE(hand);
	ASSERT_BACKTRACE(groupslist);

	if (hand->nb_groups >= 5) {
		printf("Group:\n");
		for (int j = 0; j < hand->nb_groups; ++j) {
			printf("\t%d %d\n", hand->groups[j].type, hand->groups[j].tile);
		}
		return;
	}

	if (index >= 34)
		return;

	// Check triplet group
	if (hand->histo.cells[index] >= 3) {
		struct hand handcopy;
		copy_hand(hand, &handcopy);
		add_group_hand(&handcopy, 1, TRIPLET, index);
		makegroup_rec(&handcopy, index, groupslist, pair);
	}

	// Check pair group
	if (!pair && hand->histo.cells[index] >= 2) {
		struct hand handcopy;
		copy_hand(hand, &handcopy);
		add_group_hand(&handcopy, 1, PAIR, index);
		makegroup_rec(&handcopy, index, groupslist, 1);
	}

	// Check sequence group
	if (index % 9 < 7 && index < 25 && hand->histo.cells[index] >= 1 &&
	    hand->histo.cells[index + 1] >= 1 &&
	    hand->histo.cells[index + 2] >= 1) {
		struct hand handcopy;
		copy_hand(hand, &handcopy);
		add_group_hand(&handcopy, 1, SEQUENCE, index);
		makegroup_rec(&handcopy, index, groupslist, pair);
	}

	// Check no group
	while (hand->histo.cells[index]) {
		remove_tile_hand(hand, index);
	}
	makegroup_rec(hand, index + 1, groupslist, pair);
}

// Print to stdout all possible grouplists from given hand
void makegroup(struct hand *hand, struct groupslist *groupslist) {
	struct hand handcopy;
	copy_hand(hand, &handcopy);
	makegroup_rec(&handcopy, 0, groupslist, 0);
}

struct histogram groups_to_histo(struct hand *hand) {
	struct histogram histocopy;
	copy_histogram(&hand->histo, &histocopy);
	for (int i = 0; i < 5; ++i) {
		histo_index_t tile = hand->groups[i].tile;
		switch (hand->groups[i].type) {
			case PAIR:
				add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile);
				break;
			case SEQUENCE:
				add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile + 1);
				add_histogram(&histocopy, tile + 2);
				break;
			case TRIPLET:
	                        add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile);
				break;
			case QUAD:
				add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile);
				add_histogram(&histocopy, tile);
				break;
		}
	}
	return histocopy;
}
