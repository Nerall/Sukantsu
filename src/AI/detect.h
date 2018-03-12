#ifndef _DETECT_H_
#define _DETECT_H_

#include "../core/hand.h"
#include "../core/histogram.h"

#define GROUPLIST_CAPACITY 32

struct grouplist {
	struct group groups[GROUPLIST_CAPACITY][5];
	unsigned char nb_groups;
};

void init_grouplist(struct grouplist *grouplist);

void add_copy_grouplist(struct grouplist *grouplist, struct group *group);

int ischiitoi(struct hand *hand);

int iskokushi(struct hand *hand);

int isvalid(struct hand *hand, struct grouplist *grouplist);

void makegroups(struct hand *hand, struct grouplist *grouplist);

void tenpailist(struct hand *hand, struct grouplist *grouplist);

void tilestodiscard(struct hand *hand, struct grouplist *grouplist);

void tilestocall(struct hand *hand, struct grouplist *grouplist);

void groups_to_histo(struct hand *hand, struct histogram *histocopy);

#endif // _DETECT_H_
