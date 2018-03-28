#ifndef _HAND_S_H_
#define _HAND_S_H_

#include "groups_s.h"
#include "histogram_s.h"

struct hand {
	struct histogram histo;         // tiles in hand
	struct discardlist discardlist; // list of tiles in a discard
	struct histobit chiitiles;      // tiles waiting to create a sequence
	struct histobit pontiles;       // tiles waiting to create a triplet
	struct histobit kantiles;       // tiles waiting to create a quad
	struct histobit wintiles;       // tiles waiting to win
	struct histobit furitentiles;   // tiles you already discarded
	struct histobit riichitiles; // tiles you have to discard to be tenpai (hand
	                             // > 13 tiles without considering quads)
	struct group groups[HAND_NB_GROUPS]; // each revealed group
	histo_index_t last_tile : 7;         // current tile drawn
	unsigned char nb_groups : 3;         // number of groups revealed (<= 5)
	unsigned char tenpai : 1;            // boolean, is waiting for winning
	unsigned char closed : 1;            // boolean, are only hidden groups
	unsigned char has_claimed : 1;       // boolean, tell if claimed this turn
	enum riichi_state riichi;
};

#endif // _HAND_S_H_
