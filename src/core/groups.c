#include "groups.h"
#include "../debug.h"
#include "groups_s.h"
#include <string.h>
#include "histogram.h"


// Initialize an empty group
// The pointer's data must be accessible
void init_group(struct group *group) {
	ASSERT_BACKTRACE(group);

	group->tile = NO_TILE_INDEX;
}

// Initialize the grouplist
void init_grouplist(struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	grouplist->nb_groups = 0;
}

// Add a copy of group to the grouplist
void add_copy_grouplist(struct grouplist *grouplist,
                        const struct group *group) {
	ASSERT_BACKTRACE(grouplist);
	ASSERT_BACKTRACE(group);
	ASSERT_BACKTRACE(grouplist->nb_groups < GROUPLIST_CAPACITY);

	memcpy(grouplist->groups[grouplist->nb_groups++], group,
	       HAND_NB_GROUPS * sizeof(struct group));
}

void init_discardlist(struct discardlist *discardlist) {
	ASSERT_BACKTRACE(discardlist);

	discardlist->nb_discards = 0;
}

void add_discard(struct discardlist *discardlist, histo_index_t tile) {
	ASSERT_BACKTRACE(discardlist);

	discardlist->discards[discardlist->nb_discards] = tile;
	++discardlist->nb_discards;
}

histo_index_t pop_last_discard(struct discardlist *discardlist) {
	ASSERT_BACKTRACE(discardlist);

	return discardlist->discards[--discardlist->nb_discards];
}

void init_doralist(struct doralist *doralist, struct histogram *histo) {
	ASSERT_BACKTRACE(doralist);
	for (int i = 0; i < 10; i++)
		doralist->tiles[i] = random_pop_histogram(histo);
	doralist->nb_revealed = 0;
}
