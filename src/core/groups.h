#ifndef _GROUPS_H_
#define _GROUPS_H_

struct group;
struct grouplist;

void init_group(struct group *group);

void init_grouplist(struct grouplist *grouplist);

void add_copy_grouplist(struct grouplist *grouplist, struct group *group);

#endif // _GROUPS_H_
