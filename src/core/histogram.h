#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

#include "../definitions.h"

struct histobit;
struct histogram;

// Suppose that index is unsigned
#define is_valid_index(index) ((index) < HISTO_INDEX_MAX)

void init_histobit(struct histobit *histo, unsigned char bool_value);

void set_histobit(struct histobit *histo, histo_index_t index);

void clear_histobit(struct histobit *histo, histo_index_t index);

int get_histobit(const struct histobit *histo, histo_index_t index);

void copy_histobit(const struct histobit *histo, struct histobit *histocopy);

void init_histogram(struct histogram *histo, histo_cell_t nb_tiles_index);

void add_histogram(struct histogram *histo, histo_index_t index);

void remove_histogram(struct histogram *histo, histo_index_t index);

histo_index_t random_pop_histogram(struct histogram *histo);

void copy_histogram(const struct histogram *histo, struct histogram *histocopy);

#endif // _HISTOGRAM_H_
