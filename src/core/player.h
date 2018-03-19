#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "../definitions.h"
#include "hand.h"

// TODO: Include a client struct in player
// TODO: Include a server struct in riichi_engine

struct player {
	struct hand hand;
	enum player_type player_type;
};

void init_player(struct player *player, enum player_type player_type);

histo_index_t get_player_input(struct player *player, enum action *action);

#endif // _PLAYER_H_
