#include "detect.h"
#include "hand.h"
#include "histogram.h"
#include <stdio.h>
#include <stdlib.h>

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

void makegroup_(struct hand *hand, histo_index_t index, struct histogram *alonetiles, unsigned char pair) {
	hand = hand;
	index = index;
	alonetiles = alonetiles;
	pair = pair;
}

void makegroup(struct hand *hand) {
				struct histogram alonetiles;
				init_histogram(&alonetiles, 0, 4);
        makegroup_(hand, 0, &alonetiles, 0);
}
