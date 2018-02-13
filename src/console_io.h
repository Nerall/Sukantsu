#ifndef _CONSOLE_INPUT_H_
#define _CONSOLE_INPUT_H_

#include "core/histogram.h"
#include "core/hand.h"
#include "core/detect.h"

enum action {
	ACTION_RIICHI,
	ACTION_RON,
	ACTION_TSUMO,
	ACTION_DISCARD,
	ACTION_PASS,
  ACTION_CHII,
	ACTION_PON,
  ACTION_KAN
};

histo_index_t char_to_index(char family, char number);

void index_to_char(histo_index_t index, char *family, char *number);

void print_histo(struct histogram *histo);

void print_groups(struct group *groups);

void print_victory(struct hand *hand, struct grouplist *grouplist);

histo_index_t get_input(struct histogram *histo, enum action *action);

#endif // _CONSOLE_INPUT_H_
