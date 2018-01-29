#include "hand.h"
#include <assert.h>

// Initialize an empty group
// The pointer's data must be accessible
void init_group(struct group *group) {
	assert(group);

	group->hidden = 1;
	for (int i = 0; i < GROUP_NB_TILES; ++i) {
		group->tiles[i] = -1;
	}
}

// Initialize an empty hand
// The pointer's data must be accessible
void init_hand(struct hand *hand) {
	assert(hand);

	init_histogram(&hand->histo, 0);
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		init_group(&hand->groups[i]);
	}
	hand->nb_groups = 0;
	hand->last_tile = -1;
}

// Add a group to the hand
// You must ensure that all tiles are present in the hand histogram
// Will remove the tiles from the histogram and put them in the group
// If the group have less than 4 tiles, put -1 in place of the last ones
// The first tile must not be -1
// The pointer's data must be accessible
void add_group(struct hand *hand, unsigned char hidden, histo_index_t tile1,
               histo_index_t tile2, histo_index_t tile3, histo_index_t tile4) {
	// Pre-conditions
	assert(hand);
	assert(hand->nb_groups < 5);

	// Get the group and update the hand variables
	struct group *group = &hand->groups[hand->nb_groups++];
	group->hidden = hidden;

	// 1st tile
	assert(tile1 != -1);
	remove_histogram(&hand->histo, tile1);
	group->tiles[0] = tile1;

	// If only 1 tile
	if (tile2 < 0) {
		assert(tile3 < 0 && tile4 < 0);
		group->tiles[1] = group->tiles[2] = group->tiles[3] = -1;
		return;
	}

	// 2nd tile
	remove_histogram(&hand->histo, tile2);
	group->tiles[1] = tile2;

	// If only 2 tiles
	if (tile3 < 0) {
		assert(tile4 < 0);
		group->tiles[2] = group->tiles[3] = -1;
		return;
	}

	// 3rd tile
	remove_histogram(&hand->histo, tile3);
	group->tiles[2] = tile3;

	// If only 3 tiles
	if (tile4 < 0) {
		group->tiles[3] = -1;
		return;
	}

	// 4th tile
	remove_histogram(&hand->histo, tile4);
	group->tiles[3] = tile4;
}

// Add a tile to the hand histogram
// Will also update hand->last_tile
// The pointer's data must be accessible
void add_tile_hand(struct hand *hand, histo_index_t tile) {
	assert(hand);

	add_histogram(&hand->histo, tile);
	hand->last_tile = tile;
}

// Remove a tile from the hand histogram
// The pointer's data must be accessible
void remove_tile_hand(struct hand *hand, histo_index_t tile) {
	assert(hand);

	remove_histogram(&hand->histo, tile);
}
