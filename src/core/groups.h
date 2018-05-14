#ifndef _GROUPS_H_
#define _GROUPS_H_

#include "../definitions.h"

struct group;
struct grouplist;
struct discardlist;

void init_group(struct group *group);

void init_grouplist(struct grouplist *grouplist);

void add_copy_grouplist(struct grouplist *grouplist, const struct group *group);

void init_discardlist(struct discardlist *discardlist);

void add_discard(struct discardlist *discardlist, histo_index_t tile);

histo_index_t pop_last_discard(struct discardlist *discardlist);

#endif // _GROUPS_H_
