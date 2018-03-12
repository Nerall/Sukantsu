#include "detect.h"
#include "../debug.h"
#include "histogram.h"
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

int ischiitoi(struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	int pair = 0;
	for (int i = 0; i < 34; ++i) {
		if (hand->histo.cells[i] >= 2) {
			++pair;
			if (pair >= 7)
				return 1;
		}
	}
	return 0;
}

int iskokushi(struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	int TerminalsHonors[] = {0, 8, 9, 17, 18, 26, 27, 28, 29, 30, 31, 32, 33};
	int pair = 0;
	for (int i = 0; i < 13; ++i)
		if (hand->histo.cells[TerminalsHonors[i]] >= 2)
			pair = 1;
		else if (hand->histo.cells[TerminalsHonors[i]] == 0)
			return 0;
	return pair;
}

// Check if a hand is valid; cleans grouplist by calling makegroups
int isvalid(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	makegroups(hand, grouplist);

	return grouplist->nb_groups || ischiitoi(hand) || iskokushi(hand);
}

// Remove the last group of hand->groups
// Put all tiles of group in hand-histo
static void pop_last_group(struct hand *hand) {
	ASSERT_BACKTRACE(hand);
	ASSERT_BACKTRACE(hand->nb_groups > 0);

	histo_index_t tile = hand->groups[hand->nb_groups - 1].tile;
	add_histogram(&hand->histo, tile);
	switch (hand->groups[hand->nb_groups - 1].type) {
		case PAIR:
			add_histogram(&hand->histo, tile);
			break;
		case SEQUENCE:
			add_histogram(&hand->histo, tile + 1);
			add_histogram(&hand->histo, tile + 2);
			break;
		case TRIPLET:
			add_histogram(&hand->histo, tile);
			add_histogram(&hand->histo, tile);
			break;
		case QUAD:
			add_histogram(&hand->histo, tile);
			add_histogram(&hand->histo, tile);
			add_histogram(&hand->histo, tile);
			break;
		default:
			ASSERT_BACKTRACE(0 && "Group type not recognized");
	}
	hand->groups[--hand->nb_groups].tile = NO_TILE_INDEX;
}

// Recursive function of makegroups
static void makegroups_rec(struct hand *hand, histo_index_t index,
                           struct grouplist *grouplist, unsigned char pair) {
	ASSERT_BACKTRACE(hand);

	if (hand->nb_groups >= 5) {
		add_copy_grouplist(grouplist, hand->groups);
		return;
	}

	if (index >= 34)
		return;

	histo_index_t last_tile = hand->last_tile;

	// Check triplet group
	if (hand->histo.cells[index] >= 3) {
		add_group_hand(hand, 1, TRIPLET, index);
		makegroups_rec(hand, index, grouplist, pair);
		pop_last_group(hand);
		hand->last_tile = last_tile;
	}

	// Check pair group
	if (!pair && hand->histo.cells[index] >= 2) {
		add_group_hand(hand, 1, PAIR, index);
		makegroups_rec(hand, index, grouplist, 1);
		pop_last_group(hand);
		hand->last_tile = last_tile;
	}

	// Check sequence group
	if (index % 9 < 7 && index < 25 && hand->histo.cells[index] >= 1 &&
	    hand->histo.cells[index + 1] >= 1 &&
	    hand->histo.cells[index + 2] >= 1) {
		add_group_hand(hand, 1, SEQUENCE, index);
		makegroups_rec(hand, index, grouplist, pair);
		pop_last_group(hand);
		hand->last_tile = last_tile;
	}

	// Check no group
	int nb_removed = 0;
	while (hand->histo.cells[index]) {
		remove_tile_hand(hand, index);
		++nb_removed;
	}
	makegroups_rec(hand, index + 1, grouplist, pair);
	while (nb_removed--) {
		add_tile_hand(hand, index);
	}
	hand->last_tile = last_tile;
}

// Overwrite grouplist with all possible groups from hand
// Will not modify hand
void makegroups(struct hand *hand, struct grouplist *grouplist) {
	init_grouplist(grouplist);
	makegroups_rec(hand, 0, grouplist, 0);
}

void tenpailist(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(hand);

	struct histogram histofull;
	groups_to_histo(hand, &histofull);

	init_histobit(&hand->wintiles, 0);
	hand->tenpai = 0;
	histo_index_t last_tile = hand->last_tile;
	for (int j = 0; j < 34; ++j) {
		if (histofull.cells[j] < 4) {
			add_tile_hand(hand, j);
			if (isvalid(hand, grouplist)) {
				hand->tenpai = 1;
				set_histobit(&hand->wintiles, j);
			}
			remove_tile_hand(hand, j);
		}
	}
	// hand->last_tile was overwritten by add_tile_hand
	hand->last_tile = last_tile;
}

void tilestodiscard(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(hand);

	init_histobit(&hand->riichitiles, 0);
	histo_index_t last_tile = hand->last_tile;
	for (int i = 0; i < 34; ++i) {
		if (hand->histo.cells[i]) {
			remove_tile_hand(hand, i);
			tenpailist(hand, grouplist);
			if (hand->tenpai)
				set_histobit(&hand->riichitiles, i);
			add_tile_hand(hand, i);
		}
	}
	// hand->last_tile was overwritten by add_tile_hand
	hand->last_tile = last_tile;

	// To reset hand->tenpai and hand->wintiles flags
	tenpailist(hand, grouplist);
}

void tilestocall(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(hand);

	init_histobit(&hand->chiitiles, 0);
	init_histobit(&hand->pontiles, 0);
	init_histobit(&hand->kantiles, 0);
	if (grouplist->nb_groups >= 5)
		return;

	for (int i = 0; i < 34; ++i) {
		if (hand->histo.cells[i] >= 2) {
			set_histobit(&hand->pontiles, i);
			if (hand->histo.cells[i] >= 3)
				set_histobit(&hand->kantiles, i);
		}
		if (i < 27 && i % 9 < 7) {
			if (hand->histo.cells[i + 1] && hand->histo.cells[i + 2])
				set_histobit(&hand->chiitiles, i);
			if (hand->histo.cells[i]) {
				if (hand->histo.cells[i + 2])
					set_histobit(&hand->chiitiles, i + 1);
				if (hand->histo.cells[i + 1])
					set_histobit(&hand->chiitiles, i + 2);
			}
		}
	}
}

// Copy all tile of hand (histo + groups) to histocopy
void groups_to_histo(struct hand *hand, struct histogram *histocopy) {
	ASSERT_BACKTRACE(hand);
	ASSERT_BACKTRACE(histocopy);

	copy_histogram(&hand->histo, histocopy);
	for (int i = 0; i < hand->nb_groups; ++i) {
		histo_index_t tile = hand->groups[i].tile;
		add_histogram(histocopy, tile);
		switch (hand->groups[i].type) {
			case PAIR:
				add_histogram(histocopy, tile);
				break;
			case SEQUENCE:
				add_histogram(histocopy, tile + 1);
				add_histogram(histocopy, tile + 2);
				break;
			case TRIPLET:
				add_histogram(histocopy, tile);
				add_histogram(histocopy, tile);
				break;
			case QUAD:
				add_histogram(histocopy, tile);
				add_histogram(histocopy, tile);
				add_histogram(histocopy, tile);
				break;
			default:
				ASSERT_BACKTRACE(0 && "Group type not recognized");
		}
	}
}
