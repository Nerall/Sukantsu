#include "histogram.h"
#include "../debug.h"
#include <stdlib.h>

// Initialize the histobit at the given bool value
// The pointer's data must be accessible
void init_histobit(struct histobit *histo, unsigned char bool_value) {
	ASSERT_BACKTRACE(histo);

	if (bool_value) {
		bool_value = 255;
	}

	for (int i = 0; i < 5; ++i) {
		histo->cells[i] = bool_value;
	}
}

// Set the given index to 1
// The pointer's data must be accessible
void set_histobit(struct histobit *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_tile(index));

	histo->cells[index / 8] |= (1 << (index % 8));
}

// Clear the given index (set to 0)
// The pointer's data must be accessible
void clear_histobit(struct histobit *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_tile(index));

	histo->cells[index / 8] &= ~(1 << (index % 8));
}

// Get the given index value (0 or 1)
// The pointer's data must be accessible
int get_histobit(struct histobit *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_tile(index));

	return (histo->cells[index / 8] & (1 << (index % 8))) != 0;
}

// Initialize the histogram with copies of each index
// The pointer's data must be accessible
void init_histogram(struct histogram *histo, histo_cell_t nb_tiles_index,
                    histo_cell_t max_size) {
	ASSERT_BACKTRACE(histo);
	for (int i = 0; i < HISTO_INDEX_MAX; histo->cells[i++] = nb_tiles_index)
		;
	histo->nb_tiles = nb_tiles_index * HISTO_INDEX_MAX;
	histo->max_size = max_size;
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
