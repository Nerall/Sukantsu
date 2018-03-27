#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "../definitions.h"
#include "../network/net_client.h"
#include "hand.h"

struct player {
	struct hand hand;
	enum player_type player_type;
	enum table_pos player_pos;

	struct net_client client;
	unsigned char net_id : 2;     // [0, 3]
	unsigned char net_status : 1; // bool: 1 = OK
};

void init_player(struct player *player, enum player_type player_type,
                 enum table_pos player_pos);

int player_turn(struct player *player, struct grouplist *grouplist,
                histo_index_t *index_rem);

void get_player_input(struct player *player, struct action_input *input);

void client_main_loop(struct player *player);

#endif // _PLAYER_H_
