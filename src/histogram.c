#include "histogram.h"
#include <assert.h>

// Initialize the histogram with copies of each index
// The pointer needs to be allocated
void init_histogram(struct histogram *histo, histo_cell_t nb) {
	assert(histo);

	for (int i = 0; i < NB_CELLS; histo->cells[i++] = nb)
		;
}

// Add a copy of the index in the histogram
// The pointer needs to be allocated
void add_histogram(struct histogram *histo, int index) {
	assert(histo);
	assert(index >= 0 && index < NB_CELLS);

	histo->cells[index]++;
}

// Remove a copy of the index in the histogram
// The value of the index in the histogram must be > 0
// The pointer needs to be allocated
void remove_histogram(struct histogram *histo, int index) {
	assert(histo);
	assert(index >= 0 && index < NB_CELLS);
	assert(histo->cells[index] > 0);

	histo->cells[index]--;
}
