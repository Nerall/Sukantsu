#ifndef _HISTOGRAM_S_H_
#define _HISTOGRAM_S_H_

#include "../definitions.h"

struct histobit {
	unsigned char cells[5];
};

struct histogram {
	histo_cell_t cells[HISTO_INDEX_MAX];
	unsigned char nb_tiles;
};

#endif // _HISTOGRAM_S_H_
