#ifndef _RIICHI_ENGINE_
#define _RIICHI_ENGINE_

#include "hand.h"

#define NB_PLAYERS 4
#define AI_MODE 1 // For AI debug and tests

enum game_phase {
	PHASE_INIT,
	PHASE_DRAW,
	PHASE_GETINPUT,
	PHASE_TSUMO,
	PHASE_WAIT,
	PHASE_CLAIM
};

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
