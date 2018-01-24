#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

#define NB_CELLS 34
typedef unsigned char histo_cell_t;

struct histogram {
	histo_cell_t cells[NB_CELLS];
};

void init_histogram(struct histogram *histo, histo_cell_t value);

void add_histogram(struct histogram *histo, int index);

void remove_histogram(struct histogram *histo, int index);

#endif // _HISTOGRAM_H_
