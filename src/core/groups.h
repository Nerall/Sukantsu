#ifndef _GROUPS_H_
#define _GROUPS_H_

#include "../definitions.h"

struct group {
	unsigned char hidden : 1; // boolean, is the group hidden
	histo_index_t tile : 7;   // first tile in group
	enum group_type type;
};

void init_group(struct group *group);

struct grouplist {
	struct group groups[GROUPLIST_CAPACITY][HAND_NB_GROUPS];
	unsigned char nb_groups;
};

struct discardlist {
  histo_index_t discards[24]; // max number of discards
  unsigned char nb_discards;
};

void init_grouplist(struct grouplist *grouplist);

void add_copy_grouplist(struct grouplist *grouplist, struct group *group);

void init_discardlist(struct discardlist *discardlist);

void add_discard(struct discardlist *discardlist, histo_index_t tile);

#endif // _GROUPS_H_
