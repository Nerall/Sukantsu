#ifndef _RIICHI_ENGINE_
#define _RIICHI_ENGINE_

#include "../definitions.h"
#include "groups.h"
#include "histogram.h"
#include "player.h"

struct riichi_engine {
	struct histogram wall;
	struct player players[NB_PLAYERS];
	struct grouplist grouplist;

	unsigned nb_games;
	enum game_phase phase;
};

void init_riichi_engine(struct riichi_engine *engine, enum player_type t1,
                        enum player_type t2, enum player_type t3,
                        enum player_type t4);

void play_riichi_game(struct riichi_engine *engine);

#endif // _RIICHI_ENGINE_
