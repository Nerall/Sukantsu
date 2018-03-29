#ifndef _CONSOLE_INPUT_H_
#define _CONSOLE_INPUT_H_

#include "definitions.h"

struct hand;
struct histogram;
struct group;
struct grouplist;
struct riichi_engine;

histo_index_t char_to_index(char family, char number);

void index_to_char(histo_index_t index, char *family, char *number);

void print_histo(struct histogram *histo, histo_index_t last_tile);

void print_groups(struct group *groups);

void print_victory(struct hand *hand, struct grouplist *grouplist);

histo_index_t get_input(struct histogram *histo, enum action *action);

void display_riichi(struct riichi_engine *engine, int current_player);

#endif // _CONSOLE_INPUT_H_
