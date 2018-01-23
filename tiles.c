#include "tiles.h"
#include <stdlib.h>

// Return the number of tiles in the histogram array
// The histogram array needs to be HISTO_LENGTH values long
static int get_nb_tiles_histo(histo_tile_t *array) {
	int count = 0;
	for (int i = 0; i < HISTO_LENGTH; ++i) {
		count += array[i];
	}
	return count;
}

// Create a tileset histogram of HISTO_LENGTH tiles
// Create [value] tiles for each kind
// e.g: default=4 => 4*HISTO_LENGTH tiles in total in possession
// Returned pointer can be freed
histo_tile_t *create_histo(histo_tile_t value) {
	histo_tile_t *tileset = malloc(HISTO_LENGTH * sizeof(histo_tile_t));
	for (int i = 0; i < HISTO_LENGTH; ++i) {
		tileset[i] = value;
	}
	return tileset;
}

// Create a tile array from a tileset histogram
// The histogram array needs to be HISTO_LENGTH values long
// [array_len] argument will be overewritten with the length of the array
// Returned pointer can be freed
struct tile *histo_to_array(histo_tile_t *array, int *array_len) {
	*array_len = get_nb_tiles_histo(array);
	struct tile *new_array = malloc(*array_len * sizeof(struct tile));
	int new_i = 0;
	for (int i = 0; i < HISTO_LENGTH; ++i) {
		for (int k = 0; k < array[i]; ++k) {
			index_to_tile(i, &new_array[new_i++]);
		}
	}
	return new_array;
}

// Fill a tile informations from an histo_tile index
// [tile] argument needs to be allocated
void index_to_tile(int index, struct tile *tile) {
	tile->type = index / 9;
	tile->number = index % 9;
}

// Get the histo_tile index of a tile
// [tile] argument needs to be allocated
int tile_to_index(struct tile *tile) {
	return tile->type * 9 + tile->number;
}
