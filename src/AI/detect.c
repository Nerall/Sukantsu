#include "detect.h"
#include "../core/hand.h"
#include "../core/hand_s.h"
#include "../core/histogram.h"
#include "../core/groups.h"
#include "../debug.h"

int ischiitoi(const struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	int pair = 0;
	for (histo_index_t i = 0; i < 34; ++i) {
		if (hand->histo.cells[i] >= 2) {
			++pair;
			if (pair >= 7)
				return 1;
		}
	}
	return 0;
}

int iskokushi(const struct hand *hand) {
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
int is_valid_hand(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	makegroups(hand, grouplist);

	return grouplist->nb_groups || ischiitoi(hand) || iskokushi(hand);
}

// Recursive function of makegroups
static void makegroups_rec(struct hand *hand, histo_index_t index,
                           struct grouplist *grouplist, int pair) {
	ASSERT_BACKTRACE(hand);

	if (hand->nb_groups >= 5) {
		add_copy_grouplist(grouplist, hand->groups);
		return;
	}

	for (; index < 34; ++index) {
		histo_cell_t *cur_cell = &hand->histo.cells[index];

		if (*cur_cell == 0)
			continue;

		// Check sequence group
		if (index % 9 < 7 && index < 25 && *(cur_cell + 1) && *(cur_cell + 2)) {
			add_group_hand(hand, 1, SEQUENCE, index);
			makegroups_rec(hand, index, grouplist, pair);
			pop_last_group(hand);
		}

		if (*cur_cell >= 2) {
			// Check pair group
			if (!pair) {
				add_group_hand(hand, 1, PAIR, index);
				makegroups_rec(hand, index + 1, grouplist, 1);
				pop_last_group(hand);
			}

			// Check triplet group
			if (*cur_cell >= 3) {
				add_group_hand(hand, 1, TRIPLET, index);
				makegroups_rec(hand, index + 1, grouplist, pair);
				pop_last_group(hand);
			}
		}
	}
}

// Overwrite grouplist with all possible groups from hand
// Will not modify hand
void makegroups(struct hand *hand, struct grouplist *grouplist) {
	histo_index_t last_tile = hand->last_tile;
	init_grouplist(grouplist);
	makegroups_rec(hand, 0, grouplist, 0);
	hand->last_tile = last_tile;
}

void tenpailist(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(hand);

	struct histogram histofull;
	groups_to_histo(hand, &histofull);

	init_histobit(&hand->wintiles, 0);
	hand->tenpai = 0;
	histo_index_t last_tile = hand->last_tile;
	for (histo_index_t j = 0; j < 34; ++j) {
		if (histofull.cells[j] < 4) {
			add_tile_hand(hand, j);
			if (is_valid_hand(hand, grouplist)) {
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
	for (histo_index_t i = 0; i < 34; ++i) {
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

	for (histo_index_t i = 0; i < 34; ++i) {
		histo_cell_t *cur_cell = &hand->histo.cells[i];
		if (*cur_cell >= 2) {
			set_histobit(&hand->pontiles, i);
			if (*cur_cell >= 3) {
				set_histobit(&hand->kantiles, i);
			}
		}
		if (i < 27 && i % 9 < 7) {
			if (*(cur_cell + 1) && *(cur_cell + 2))
				set_histobit(&hand->chiitiles, i);
			if (*cur_cell) {
				if (*(cur_cell + 2))
					set_histobit(&hand->chiitiles, i + 1);
				if (*(cur_cell + 1))
					set_histobit(&hand->chiitiles, i + 2);
			}
		}
	}
}

// Copy all tile of hand (histo + groups) to histocopy
void groups_to_histo(const struct hand *hand, struct histogram *histocopy) {
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
