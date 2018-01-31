#ifndef _HAND_H_
#define _HAND_H_

#include "histogram.h"

enum group_type { PAIR, SEQUENCE, TRIPLET, QUAD };
enum riichi_state { RIICHI, IPPATSU, DOUBLE_RIICHI, DOUBLE_IPPATSU };

#define HAND_NB_GROUPS 5
#define GROUP_NB_TILES 4
#define NO_TILE_INDEX 127

struct group {
	unsigned char hidden : 1; // boolean, is the group hidden
	histo_index_t tile : 7;   // first tile in group
	enum group_type type;
};

void init_group(struct group *group);

struct hand {
	struct histogram histo;              // tiles in hand
	struct histogram discarded_tiles;    // each tiles discarded
	struct histobit chiitiles;           // tiles waiting to create a sequence
	struct histobit pontiles;            // tiles waiting to create a triplet
	struct histobit kantiles;            // tiles waiting to create a quad
	struct histobit wintiles;            // tiles waiting to win
	struct group groups[HAND_NB_GROUPS]; // each revealed group
	histo_index_t last_tile;             // current tile drawn
	unsigned char nb_groups : 1;         // boolean, number of groups revealed
	unsigned char tenpai : 1;            // boolean, is waiting for winning
	unsigned char closed : 1;            // boolean, are only hidden groups
};

void init_hand(struct hand *hand);

void add_group_hand(struct hand *hand, unsigned char hidden,
                    enum group_type type, histo_index_t tile);

void add_tile_hand(struct hand *hand, histo_index_t tile);

void remove_tile_hand(struct hand *hand, histo_index_t tile);

void copy_hand(struct hand *hand, struct hand *handcopy);

#endif // _HAND_H_
