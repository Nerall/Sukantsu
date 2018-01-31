#include "histogram.h"
#include "../debug.h"
#include <stdlib.h>

// Initialize the histogram with copies of each index
// The pointer's data must be accessible
void init_histogram(struct histogram *histo, histo_cell_t nb_tiles_index) {
	ASSERT_BACKTRACE(histo);
	for (int i = 0; i < HISTO_INDEX_MAX; histo->cells[i++] = nb_tiles_index)
		;
	histo->nb_tiles = nb_tiles_index * HISTO_INDEX_MAX;
	//histo->max_size = max_size;
}

// Add a copy of the index in the histogram
// The pointer's data must be accessible
void add_histogram(struct histogram *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_tile(index));

	histo->cells[index]++;
}

// Remove a copy of the index in the histogram
// The value of the index in the histogram must be > 0
// The pointer's data must be accessible
void remove_histogram(struct histogram *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_tile(index));
	ASSERT_BACKTRACE(histo->cells[index] > 0);

	histo->cells[index]--;
}

// Pop a random index in the histogram
// The pointer needs to be allocated
histo_index_t random_pop_histogram(struct histogram *histo) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(histo->nb_tiles > 0);

	// We get a random number in [1, nb_tiles]
	int r = 1 + (rand() % histo->nb_tiles);
	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		r -= histo->cells[i];
		if (r <= 0) {
			remove_histogram(histo, i);
			histo->nb_tiles--;
			return i;
		}
	}

	ASSERT_BACKTRACE(0 && "random_pop_wall: histogram - out of bounds");
}

void copy_histogram(struct histogram *histo, struct histogram *histocopy) {
	assert(histo);
	assert(histocopy);
	for (int i = 0; i < 34; ++i) {
		histocopy->cells[i] = histo->cells[i];
	}
	histocopy->max_size = histo->max_size;
	histocopy->nb_tiles = histo->nb_tiles;
}
