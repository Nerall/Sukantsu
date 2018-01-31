#ifndef _DETECT_H_
#define _DETECT_H_

#include "hand.h"
#include "hand.h"
#include "histogram.h"
#include <stdio.h>

int isclassical(struct hand *hand);
int ischiitoi(struct hand *hand);
int iskokushi(struct hand *hand);
int isvalid(struct hand *hand);
void makegroup_(struct hand *hand, int index, struct histogram *alonetiles, unsigned char pair);
void makegroup(struct hand *hand);

#endif // _DETECT_H_
