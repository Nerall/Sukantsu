#ifndef _WALL_H_
#define _WALL_H_

#include "histogram.h"

struct wall {
	struct histogram histo;
	int nb_tiles;
};

void init_wall(struct wall *wall);

histo_index_t random_pop_wall(struct wall *wall);

#endif // _WALL_H_
