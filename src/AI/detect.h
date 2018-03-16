#ifndef _DETECT_H_
#define _DETECT_H_

#include "../core/hand.h"
#include "../core/groups.h"
#include "../core/histogram.h"

int ischiitoi(struct hand *hand);

int iskokushi(struct hand *hand);

int is_valid_hand(struct hand *hand, struct grouplist *grouplist);

void makegroups(struct hand *hand, struct grouplist *grouplist);

void tenpailist(struct hand *hand, struct grouplist *grouplist);

void tilestodiscard(struct hand *hand, struct grouplist *grouplist);

void tilestocall(struct hand *hand, struct grouplist *grouplist);

void groups_to_histo(struct hand *hand, struct histogram *histocopy);

#endif // _DETECT_H_
