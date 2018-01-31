#include "histogram.h"
#include <assert.h>

// Initialize the histogram with copies of each index
// The pointer's data must be accessible
void init_histogram(struct histogram *histo, histo_cell_t nb_tiles, histo_cell_t max_size) {
	assert(histo);
	for (int i = 0; i < HISTO_INDEX_MAX; histo->cells[i++] = nb_tiles)
		;
	histo->nb_tiles = nb_tiles;
	histo->max_size = max_size;
}

// Add a copy of the index in the histogram
// The pointer's data must be accessible
void add_histogram(struct histogram *histo, histo_index_t index) {
	assert(histo);
	assert(index >= 0 && index < HISTO_INDEX_MAX);

	histo->cells[(unsigned char)index]++;
}

// Remove a copy of the index in the histogram
// The value of the index in the histogram must be > 0
// The pointer's data must be accessible
void remove_histogram(struct histogram *histo, histo_index_t index) {
	assert(histo);
	assert(index >= 0 && index < HISTO_INDEX_MAX);
	assert(histo->cells[(unsigned char)index] > 0);

	histo->cells[(unsigned char)index]--;
}
