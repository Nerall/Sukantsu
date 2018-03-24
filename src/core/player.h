#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "../definitions.h"
#include "hand.h"

// TODO: Include a client struct in player
// TODO: Include a server struct in riichi_engine

struct action_input {
	enum action action;
	histo_index_t tile;
};

struct player {
	struct hand hand;
	enum player_type player_type;
	enum table_pos player_pos;
	int net_id;
};

void init_player(struct player *player, enum player_type player_type,
                 enum table_pos player_pos);

int player_turn(struct player *player, struct grouplist *grouplist,
                histo_index_t *index_rem);

void get_player_input(struct player *player, struct action_input *input);

#endif // _PLAYER_H_
