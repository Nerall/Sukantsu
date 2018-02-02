#ifndef _DETECT_H_
#define _DETECT_H_

#include "hand.h"
#include "histogram.h"

struct groupslist {
  struct grouplist *next;
  struct group *groups[];
};

int isclassical(struct hand *hand);
int ischiitoi(struct hand *hand);
int iskokushi(struct hand *hand);
int isvalid(struct hand *hand);
void makegroup_(struct hand *hand, int index, struct histogram *alonetiles);
void makegroup(struct hand *hand, struct groupslist *groupslist);
struct histogram groups_to_histo(struct hand *hand);

#endif // _DETECT_H_
