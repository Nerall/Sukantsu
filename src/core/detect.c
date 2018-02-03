#include "detect.h"
#include "../debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the grouplist
void init_grouplist(struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	grouplist->nb_groups = 0;
}

// Add a copy of group to the grouplist
void add_copy_grouplist(struct grouplist *grouplist, struct group *group) {
	ASSERT_BACKTRACE(grouplist);
	ASSERT_BACKTRACE(group);
	ASSERT_BACKTRACE(grouplist->nb_groups < GROUPLIST_CAPACITY);
	memcpy(grouplist->groups[grouplist->nb_groups++], group,
	       HAND_NB_GROUPS * sizeof(struct group));
}
/* //TO DELETE
static int isclassical(struct hand *hand) {
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
*/
static int ischiitoi(struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	for (int i = 0; i < 34; ++i) {
		if (hand->histo.cells[i] != 0 && hand->histo.cells[i] != 2)
			return 0;
	}
	return 1;
}

static int iskokushi(struct hand *hand) {
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

// Check if a hand is valid; cleans grouplist by calling makegroups
int isvalid(struct hand *hand, struct grouplist *grouplist) {
  makegroups(hand, grouplist);
  printf("%u\n", grouplist->nb_groups);
	return grouplist->nb_groups || ischiitoi(hand) || iskokushi(hand);
}

// Recursive function of makegroups
static void makegroups_rec(struct hand *hand, int index,
                           struct grouplist *grouplist, unsigned char pair) {
	ASSERT_BACKTRACE(hand);

	if (hand->nb_groups >= 5) {
		add_copy_grouplist(grouplist, hand->groups);
		for (int g = 0; g < 5; ++g) {
      printf("\t(%d, %d, %d)\n", hand->groups[g].hidden, hand->groups[g].type, hand->groups[g].tile);
    }
    //printf("Group found!\n");
		return;
	}

	if (index >= 34)
		return;

	// Check triplet group
	if (hand->histo.cells[index] >= 3) {
		struct hand handcopy;
		copy_hand(hand, &handcopy);
		add_group_hand(&handcopy, 1, TRIPLET, index);
		makegroups_rec(&handcopy, index, grouplist, pair);
	}

	// Check pair group
	if (!pair && hand->histo.cells[index] >= 2) {
		struct hand handcopy;
		copy_hand(hand, &handcopy);
		add_group_hand(&handcopy, 1, PAIR, index);
		makegroups_rec(&handcopy, index, grouplist, 1);
	}

	// Check sequence group
	if (index % 9 < 7 && index < 25 && hand->histo.cells[index] >= 1 &&
	    hand->histo.cells[index + 1] >= 1 &&
	    hand->histo.cells[index + 2] >= 1) {
		struct hand handcopy;
		copy_hand(hand, &handcopy);
		add_group_hand(&handcopy, 1, SEQUENCE, index);
		makegroups_rec(&handcopy, index, grouplist, pair);
	}

	// Check no group
	while (hand->histo.cells[index]) {
		remove_tile_hand(hand, index);
	}
	makegroups_rec(hand, index + 1, grouplist, pair);
}

// Overwrite grouplist with all possible groups from hand
void makegroups(struct hand *hand, struct grouplist *grouplist) {
	struct hand handcopy;
	copy_hand(hand, &handcopy);
	init_grouplist(grouplist);
	makegroups_rec(&handcopy, 0, grouplist, 0);
}

void tenpailist(struct hand *hand, struct grouplist *grouplist) {
  struct histogram histofull = groups_to_histo(hand);
  init_histobit(&hand->wintiles, 0);
  for (int i = 0; i < 34; ++i) {
    struct hand handcopy;
    copy_hand(hand, &handcopy);
    if (histofull.cells[i] != 4) {
      add_tile_hand(&handcopy, i);
      for (int w = 0; w < 34; ++w) {
        printf("%u", hand->wintiles.cells[w]);
      }
      printf("%u\n", handcopy.nb_groups);
      if (isvalid(&handcopy, grouplist)) {
        hand->tenpai = 1;
        hand->wintiles.cells[i] = 255;
      }
    } 
  }
}

struct histogram groups_to_histo(struct hand *hand) {
	struct histogram histocopy;
	copy_histogram(&hand->histo, &histocopy);
	for (int i = 0; i < hand->nb_groups; ++i) {
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
