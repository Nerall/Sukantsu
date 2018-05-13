#ifndef _DETECT_H_
#define _DETECT_H_

struct hand;
struct grouplist;
struct histogram;

int ischiitoi(const struct hand *hand);

int iskokushi(const struct hand *hand);

int is_valid_hand(struct hand *hand, struct grouplist *grouplist);

int makegroups(struct hand *hand, struct grouplist *grouplist);

void tenpailist(struct hand *hand, struct grouplist *grouplist);

void tilestodiscard(struct hand *hand, struct grouplist *grouplist);

void tilestocall(struct hand *hand, const struct grouplist *grouplist);

void set_hand_histobits(struct hand *hand, struct grouplist *grouplist);

void groups_to_histo(const struct hand *hand, struct histogram *histocopy);

#endif // _DETECT_H_
