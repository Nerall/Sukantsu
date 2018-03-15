#include "groups.h"
#include "../debug.h"
#include <string.h>

// Initialize an empty group
// The pointer's data must be accessible
void init_group(struct group *group) {
	ASSERT_BACKTRACE(group);

	group->hidden = 1;
	group->type = -1;
	group->tile = NO_TILE_INDEX;
}

// Initialize the grouplist
void init_grouplist(struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	grouplist->nb_groups = 0;
}

// Add a copy of group to the grouplist
void add_copy_grouplist(struct grouplist *grouplist, struct group *group) {
	ASSERT_BACKTRACE(grouplist);
	ASSERT_BACKTRACE(group);
	ASSERT_BACKTRACE(grouplist->nb_groups < GROUPLIST_CAPACITY);

	memcpy(grouplist->groups[grouplist->nb_groups++], group,
	       HAND_NB_GROUPS * sizeof(struct group));
}
