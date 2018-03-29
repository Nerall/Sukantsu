#ifndef _RIICHI_ENGINE_
#define _RIICHI_ENGINE_

#include "../definitions.h"

struct riichi_engine;

void init_riichi_engine(struct riichi_engine *engine, enum player_type t1,
                        enum player_type t2, enum player_type t3,
                        enum player_type t4);

int play_riichi_game(struct riichi_engine *engine);

#endif // _RIICHI_ENGINE_
