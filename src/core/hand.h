#ifndef _HAND_H_
#define _HAND_H_

#include "histogram.h"

enum group_type { PAIR, SEQUENCE, TRIPLET, QUAD };
enum riichi_state { RIICHI, IPPATSU, DOUBLE_RIICHI, DOUBLE_IPPATSU };

#define HAND_NB_GROUPS 5
#define GROUP_NB_TILES 4

struct group {
	unsigned char hidden; //boolean, is the group hidden
	enum group_type type;
	histo_index_t tile; //first tile in group

};

void init_group(struct group *group);

struct hand {
	struct histogram histo; //tiles in hand
	struct histogram chiitiles; //tiles waiting to create a sequence
	struct histogram pontiles; //tiles waiting to create a triplet
	struct histogram kantiles; //tiles waiting to create a quad
	struct histogram wintiles; //tiles waiting to win
	struct histogram discarded_tiles; //each tiles discarded
	struct group groups[HAND_NB_GROUPS]; //each revealed group
	histo_index_t last_tile; //current tile drawn
	unsigned char nb_groups; //boolean, number of groups revealed
	unsigned char tenpai; //boolean, is waiting for winning
	unsigned char closed; //boolean, are only hidden groups

};

void init_hand(struct hand *hand);

void add_group_hand(struct hand *hand, unsigned char hidden,
                    enum group_type type, histo_index_t tile);

void add_tile_hand(struct hand *hand, histo_index_t tile);

void remove_tile_hand(struct hand *hand, histo_index_t tile);

void copy_hand(struct hand *hand, struct hand *handcopy);

#endif // _HAND_H_
