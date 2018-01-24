#include <assert.h>
#include <stdlib.h>
#include "wall.h"

#define NB_COPY_TILES 4

// Initialize the wall structure
// The pointer needs to be allocated
void init_wall(struct wall *wall) {
	assert(wall);

	init_histogram(&wall->histo, NB_COPY_TILES);
	wall->nb_tiles = NB_COPY_TILES * NB_CELLS;
}

// Pop a random index in the histogram
// The pointer needs to be allocated
int random_pop_wall(struct wall *wall) {
	assert(wall);
	assert(wall->nb_tiles > 0);

	// We get a random number in [1, nb_tiles]
	int r = 1 + (rand() % wall->nb_tiles);
	for (int i = 0; i < NB_CELLS; ++i) {
		r -= wall->histo.cells[i];
		if (r <= 0) {
			remove_histogram(&wall->histo, i);
			wall->nb_tiles--;
			return i;
		}
	}

	assert(0 && "random_pop_wall: histogram - out of bounds");
}
