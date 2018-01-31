#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

#define HISTO_INDEX_MAX 34
// Suppose that index is unsigned
#define is_valid_tile(index) ((index) < HISTO_INDEX_MAX)

typedef char histo_cell_t;
typedef unsigned char histo_index_t;

struct histogram {
	histo_cell_t cells[HISTO_INDEX_MAX];
	//histo_cell_t max_size;
	unsigned char nb_tiles;
};

void init_histogram(struct histogram *histo, histo_cell_t nb_tiles_index);

void add_histogram(struct histogram *histo, histo_index_t index);

void remove_histogram(struct histogram *histo, histo_index_t index);

histo_index_t random_pop_histogram(struct histogram *histo);

#endif // _HISTOGRAM_H_
