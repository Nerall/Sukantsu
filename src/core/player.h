#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "../definitions.h"

struct player;
struct group;
struct grouplist;
struct net_client;

void init_player(struct player *player, enum player_type player_type,
                 enum table_pos player_pos);

int player_turn(struct player *player, struct grouplist *grouplist,
                histo_index_t *index_rem);

void apply_call(struct player *player, const struct action_input *input);

void get_player_input(struct player *player, struct action_input *input);

void client_main_loop(struct net_client *client);

#endif // _PLAYER_H_
