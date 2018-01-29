#include "detect.h"
#include <stdlib.h>
#include <stdio.h>

int isclassical(struct hand *hand) {
	if (hand->groups[4].type != 0) {
		pair = 0;
		for (int i = 0; i < 5; ++i)
			if (hand->groups[i].type == 'PAIR' && !pair)
				pair = 1;
			else if (hand->groups[i].type == 'PAIR')
				return 0;
		return 1;
	}
	else
		return 0;
}

int ischiitoi(struct hand *hand) {
	for (histo_size_t i = 0; i < 14; ++i) {
		if (hand->histo[i] != 0 && hand->histo[i] != 2)
			return 0;
	}
	return 1;
}

int iskokushi(struct hand *hand) {
	histo_size_t[] TerminalsHonors = [0,8,9,17,18,26,27,28,29,30,31,32,33];
	pair = 0;
	for (int i = 0, i < 13; ++i)
		if (hand->histo[TerminalsHonors[i]] == 2 && !pair)
			pair = 1;
		else if (hand->histo[i] != 1)
			return 0;
	return 1;
}

int isvalid(struct hand *hand) {
	return isclassical(hand) || ischiitoi(hand) || iskokushi(hand)
}
