#ifndef _GROUPS_S_H_
#define _GROUPS_S_H_

#include "../definitions.h"

struct group {
	unsigned char hidden : 1; // boolean, is the group hidden
	histo_index_t tile : 7;   // first tile in group
	enum group_type type;
};

struct grouplist {
	struct group groups[GROUPLIST_CAPACITY][HAND_NB_GROUPS];
	unsigned char nb_groups;
};

struct discardlist {
	histo_index_t discards[24]; // max number of discards
	unsigned char nb_discards;
};

struct doralist {
	histo_index_t tiles[10];
	unsigned char nb_revealed;
};
#endif // _GROUPS_S_H_
