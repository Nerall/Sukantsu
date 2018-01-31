#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

#define HISTO_INDEX_MAX 34
typedef char histo_cell_t;
typedef char histo_index_t;

struct histogram {
	histo_cell_t cells[HISTO_INDEX_MAX];
	histo_cell_t max_size;
	unsigned char nb_tiles;
};

void init_histogram(struct histogram *histo, histo_cell_t nb_tiles, histo_cell_t max_size);

void add_histogram(struct histogram *histo, histo_index_t index);

void remove_histogram(struct histogram *histo, histo_index_t index);

#endif // _HISTOGRAM_H_
