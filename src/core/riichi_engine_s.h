#ifndef _RIICHI_ENGINE_S_H
#define _RIICHI_ENGINE_S_H

#include "../network/net_server_s.h"
#include "groups_s.h"
#include "histogram_s.h"
#include "player_s.h"
#include "../console_io.h"

struct riichi_engine {
	struct histogram wall;
	struct player players[NB_PLAYERS];
	struct grouplist grouplist;
	struct net_server server;
	struct gameGUI gameGUI;
	signed nb_games;
	signed nb_rounds;
	enum game_phase phase;
};

#endif // _RIICHI_ENGINE_S_H
