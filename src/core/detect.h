#ifndef _DETECT_H_
#define _DETECT_H_

#include "hand.h"

int isclassical(struct hand *hand);
int ischiitoi(struct hand *hand);
int iskokushi(struct hand *hand);
int isvalid(struct hand *hand);

#endif // _DETECT_H_
