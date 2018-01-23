#ifndef _TILES_H_
#define _TILES_H_

#define HISTO_LENGTH 34
typedef unsigned char histo_tile_t;

struct tile {
	unsigned char group : 2; // 4 groups => 2 bits
	unsigned char number : 4; // 10 numbers => 4 bits
};

histo_tile_t *create_histo(histo_tile_t value);

struct tile *histo_to_array(histo_tile_t *array, int *array_len);

void index_to_tile(int index, struct tile *tile);

int tile_to_index(struct tile *tile);

#endif // _TILESET_H_
