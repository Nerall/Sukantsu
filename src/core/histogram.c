#include "histogram.h"
#include "../debug.h"
#include <stdlib.h>
#include <string.h>

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
	ASSERT_BACKTRACE(is_valid_index(index));

	histo->cells[index / 8] |= (1 << (index % 8));
}

// Clear the given index (set to 0)
// The pointer's data must be accessible
void clear_histobit(struct histobit *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_index(index));

	histo->cells[index / 8] &= ~(1 << (index % 8));
}

// Get the given index value (0 or 1)
// The pointer's data must be accessible
int get_histobit(struct histobit *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_index(index));

	return (histo->cells[index / 8] & (1 << (index % 8))) != 0;
}

// Do a deep-copy of histo to histocopy
// The pointers needs to be allocated
void copy_histobit(struct histobit *histo, struct histobit *histocopy) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(histocopy);

	memcpy(histocopy, histo, sizeof(struct histobit));
}

// Initialize the histogram with copies of each index
// The pointer's data must be accessible
void init_histogram(struct histogram *histo, histo_cell_t nb_tiles_index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(nb_tiles_index <= 4);

	for (int i = 0; i < HISTO_INDEX_MAX; histo->cells[i++] = nb_tiles_index)
		;
	histo->nb_tiles = nb_tiles_index * HISTO_INDEX_MAX;
}

// Add a copy of the index in the histogram
// The pointer's data must be accessible
void add_histogram(struct histogram *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_index(index));
	ASSERT_BACKTRACE(histo->cells[index] < 4);

	histo->cells[index]++;
	histo->nb_tiles++;
}

// Remove a copy of the index in the histogram
// The value of the index in the histogram must be > 0
// The pointer's data must be accessible
void remove_histogram(struct histogram *histo, histo_index_t index) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(is_valid_index(index));
	ASSERT_BACKTRACE(histo->cells[index] > 0);

	histo->cells[index]--;
	histo->nb_tiles--;
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
			return i;
		}
	}

	ASSERT_BACKTRACE(0 && "random_pop_wall: histogram - out of bounds");
}

// Do a deep-copy of histo to histocopy
// The pointers needs to be allocated
void copy_histogram(struct histogram *histo, struct histogram *histocopy) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(histocopy);

	memcpy(histocopy, histo, sizeof(struct histogram));
}
