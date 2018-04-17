#ifndef _HAND_H_
#define _HAND_H_

#include "../definitions.h"

struct hand;

void init_hand(struct hand *hand);

void add_group_hand(struct hand *hand, unsigned char hidden,
                    enum group_type type, histo_index_t tile);

void pop_last_group(struct hand *hand);

void add_tile_hand(struct hand *hand, histo_index_t tile);

void remove_tile_hand(struct hand *hand, histo_index_t tile);

void copy_hand(const struct hand *hand, struct hand *handcopy);

#endif // _HAND_H_
