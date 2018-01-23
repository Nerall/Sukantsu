#include "tiles.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Shuffle a struct tile array in-place
static void shuffle_tiles(struct tile *array, int length) {
	for (int i = length - 1; i > 1; --i) {
		int j = rand() % (i + 1);
		struct tile temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

int main() {
	srand(time(NULL));
	printf("Sizeof(struct tile)  = %lu byte(s)\n", sizeof(struct tile));
	printf("Sizeof(histo_tile_t) = %lu byte(s)\n", sizeof(histo_tile_t));

	printf("Histogram (4 tiles each):\n");
	printf("\tSizeof(histo) = %lu bytes\n\t",
	       HISTO_LENGTH * sizeof(histo_tile_t));
	histo_tile_t *histo = create_histo(4);
	for (int i = 0; i < HISTO_LENGTH; ++i) {
		printf("%d ", histo[i]);
	}
	printf("\nOK\n");

	printf("Tile array from histogram:\n");
	int new_length;
	struct tile *array = histo_to_array(histo, &new_length);
	printf("\tlength = %d\n", new_length);
	printf("\tSizeof(array) = %lu bytes\n", new_length * sizeof(array[0]));
	for (int i = 0; i < new_length; ++i) {
		int type = array[i].type;
		printf("\tType %d: %d", type, array[i].number);
		for (i++; i < new_length && array[i].type == type; ++i) {
			printf(" %d", array[i].number);
		}
		i--;
		printf("\n");
	}
	printf("OK\n");

	printf("Shuffle the array:\n");
	shuffle_tiles(array, new_length);
	for (int i = 0; i < new_length; ++i) {
		printf("\t");
		for (int j = 0; i < new_length && j < 10; ++i, ++j) {
			printf("(%d,%d) ", array[i].type, array[i].number);
		}
		printf("\n");
	}
	printf("\nOK\n");

	printf("Freeing the memory:\n");
	free(histo);
	free(array);
	printf("OK\n");
}
