#ifndef _RIICHI_ENGINE_S_H
#define _RIICHI_ENGINE_S_H

#include "../network/net_server.h"
#include "groups_s.h"
#include "player.h"
#include "player_s.h"

struct riichi_engine {
	struct histogram wall;
	struct player players[NB_PLAYERS];
	struct grouplist grouplist;
	struct net_server server;

	unsigned nb_games;
	enum game_phase phase;
};

#endif // _RIICHI_ENGINE_S_H
