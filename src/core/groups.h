#ifndef _GROUPS_H_
#define _GROUPS_H_

#include "../definitions.h"
#include "histogram.h"

struct group;
struct grouplist;
struct discardlist;
struct doralist;

void init_group(struct group *group);

void init_grouplist(struct grouplist *grouplist);

void add_copy_grouplist(struct grouplist *grouplist, const struct group *group);

void init_discardlist(struct discardlist *discardlist);

void add_discard(struct discardlist *discardlist, histo_index_t tile);

histo_index_t pop_last_discard(struct discardlist *discardlist);

void init_doralist(struct doralist *doralist, struct histogram *histo);


#endif // _GROUPS_H_
