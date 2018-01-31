#include "detect.h"

int isclassical(struct hand *hand) {
	if (hand->groups[4].type == 0)
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
	for (int i = 0; i < 14; ++i) {
		if (hand->histo.cells[i] != 0 && hand->histo.cells[i] != 2)
			return 0;
	}
	return 1;
}

int iskokushi(struct hand *hand) {
	int TerminalsHonors[] = {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33};
	unsigned char pair = 0;
	for (int i = 0; i < 13; ++i)
		if (hand->histo.cells[TerminalsHonors[i]] == 2 && !pair)
			pair = 1;
		else if (hand->histo.cells[TerminalsHonors[i]] != 1)
			return 0;
	return 1;
}

int isvalid(struct hand *hand) {
	return isclassical(hand) || ischiitoi(hand) || iskokushi(hand);
}

void makegroup_(struct hand *hand, int index, struct histogram *alonetiles,
                unsigned char pair) {
	if (index == 34 && hand->nb_groups >= 5) {
		for (int j = 0; j < hand->nb_groups; ++j) {
			printf("%d %d\n", hand->groups[j].type, hand->groups[j].tile);
		}
	}
	else {
		if (hand->histo.cells[index] >= 3) {
			struct hand handcopy;
			copy_hand(hand, &handcopy);
			handcopy.histo.cells[index] -= 3;
			add_group_hand(&handcopy, 1, TRIPLET, index);
			makegroup_(&handcopy, index, alonetiles, pair);
		}
		if (hand->histo.cells[index] >= 2 && !pair) {
			struct hand handcopy;
			copy_hand(hand, &handcopy);
			handcopy.histo.cells[index] -= 2;
			add_group_hand(&handcopy, 1, PAIR, index);
			makegroup_(&handcopy, index, alonetiles, 1);
		}
		if (index % 9 < 7 && index < 25 && hand->histo.cells[index] >= 1 &&
				hand->histo.cells[index+1] >= 1 && hand->histo.cells[index+2] >= 1) {
			struct hand handcopy;
			copy_hand(hand, &handcopy);
			handcopy.histo.cells[index] -= 1;
			handcopy.histo.cells[index+1] -= 1;
			handcopy.histo.cells[index+2] -= 1;
			add_group_hand(&handcopy, 1, SEQUENCE, index);
			makegroup_(&handcopy, index, alonetiles, pair);
		}
		alonetiles->cells[index] += hand->histo.cells[index];
		hand->histo.cells[index] = 0;
		makegroup_(hand, index + 1, alonetiles, pair);

	}
	hand = hand;
	index = index;
	alonetiles = alonetiles;
	pair = pair;
}

void makegroup(struct hand *hand) {
	struct hand handcopy;
	copy_hand(hand, &handcopy);

	struct histogram alonetiles;
	init_histogram(&alonetiles, 0);
	makegroup_(&handcopy, 0, &alonetiles, 0);
}
