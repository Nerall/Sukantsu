#ifndef _HAND_H_
#define _HAND_H_

#include "histogram.h"

enum group_type { PAIR, SEQUENCE, TRIPLET, QUAD };

#define HAND_NB_GROUPS 5
#define GROUP_NB_TILES 4

struct group {
	unsigned char hidden;
	enum group_type type;
	histo_index_t tile;
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
                    enum group_type type, histo_index_t tile);

void add_tile_hand(struct hand *hand, histo_index_t tile);

void remove_tile_hand(struct hand *hand, histo_index_t tile);

#endif // _HAND_H_
