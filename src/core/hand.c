#include "hand.h"
#include "../debug.h"

#define NO_TILE_INDEX 255

// Initialize an empty group
// The pointer's data must be accessible
void init_group(struct group *group) {
	ASSERT_BACKTRACE(group);

	group->hidden = 1;
	group->type = 0;
	group->tile = NO_TILE_INDEX;
}

// Initialize an empty hand
// The pointer's data must be accessible
void init_hand(struct hand *hand) {
	ASSERT_BACKTRACE(hand);

	init_histogram(&hand->histo, 0, 4);
	init_histogram(&hand->chiitiles, 0, 1);
	init_histogram(&hand->pontiles, 0, 1);
	init_histogram(&hand->kantiles, 0, 1);
	init_histogram(&hand->wintiles, 0, 1);
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		init_group(&hand->groups[i]);
	}
	hand->last_tile = NO_TILE_INDEX;
	hand->nb_groups = 0;
	hand->tenpai = 0;
	hand->closed = 1;
}

// Add a group to the hand
// You must ensure that all tiles are present in the hand histogram
// Will remove the tiles from the histogram and put them in the group
// The tile must not be -1
// The pointer's data must be accessible
void add_group_hand(struct hand *hand, unsigned char hidden,
                    enum group_type type, histo_index_t tile) {
	// Pre-conditions
	ASSERT_BACKTRACE(hand);
	ASSERT_BACKTRACE(hand->nb_groups < 5);

	remove_tile_hand(hand, tile);
	switch (type) {
		case PAIR:
			remove_tile_hand(hand, tile);
			break;

		case SEQUENCE:
			remove_tile_hand(hand, tile + 1);
			remove_tile_hand(hand, tile + 2);
			break;

		case TRIPLET:
			remove_tile_hand(hand, tile);
			remove_tile_hand(hand, tile);
			break;

		case QUAD:
			remove_tile_hand(hand, tile);
			remove_tile_hand(hand, tile);
			remove_tile_hand(hand, tile);
			break;
	}

	// Get the group and update the hand variables
	struct group *group = &hand->groups[hand->nb_groups++];
	group->hidden = hidden;
	group->type = type;
	group->tile = tile;
}

// Add a tile to the hand histogram
// Will also update hand->last_tile
// The pointer's data must be accessible
void add_tile_hand(struct hand *hand, histo_index_t tile) {
	ASSERT_BACKTRACE(hand);

	add_histogram(&hand->histo, tile);
	hand->last_tile = tile;
}

// Remove a tile from the hand histogram
// The pointer's data must be accessible
void remove_tile_hand(struct hand *hand, histo_index_t tile) {
	ASSERT_BACKTRACE(hand);

	remove_histogram(&hand->histo, tile);
}
