#ifndef _HAND_H_
#define _HAND_H_

#include "histogram.h"

#define HAND_NB_GROUPS 5
#define GROUP_NB_TILES 4

struct group {
	unsigned char hidden;
	histo_index_t tiles[GROUP_NB_TILES];
};

void init_group(struct group *group);

struct hand {
	struct histogram histo;
	struct group groups[HAND_NB_GROUPS];

	unsigned char nb_groups;
	histo_index_t last_tile;
};

void init_hand(struct hand *hand);

void add_group_hand(struct hand *hand, unsigned char hidden,
                    histo_index_t tile1, histo_index_t tile2,
                    histo_index_t tile3, histo_index_t tile4);

void add_tile_hand(struct hand *hand, histo_index_t tile);

void remove_tile_hand(struct hand *hand, histo_index_t tile);

#endif // _HAND_H_
