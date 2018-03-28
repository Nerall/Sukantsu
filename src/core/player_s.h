#ifndef _PLAYER_S_H
#define _PLAYER_S_H

#include "../network/net_client.h"
#include "hand_s.h"

struct player {
	struct hand hand;
	enum player_type player_type;
	enum table_pos player_pos;

	struct net_client client;
	unsigned char net_id : 2;     // [0, 3]
	unsigned char net_status : 1; // bool: 1 = OK
};

#endif // _PLAYER_S_H
