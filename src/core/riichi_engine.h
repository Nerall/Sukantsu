#ifndef _RIICHI_ENGINE_
#define _RIICHI_ENGINE_

#include "../definitions.h"
#include "histogram.h"
#include "hand.h"
#include "groups.h"

struct riichi_engine {
	struct histogram wall;
	struct hand hands[NB_PLAYERS];
	struct grouplist grouplists[NB_PLAYERS];

	unsigned nb_games;
	enum game_phase phase;
};

void init_riichi_engine(struct riichi_engine *engine);

void play_riichi_game(struct riichi_engine *engine);

void display_riichi(struct riichi_engine *engine, int current_player);

#endif // _RIICHI_ENGINE_
