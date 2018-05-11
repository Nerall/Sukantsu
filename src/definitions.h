#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

// For AI debug and tests
#define AI_MODE 1

#define GROUPLIST_CAPACITY 32
#define GROUP_NB_TILES 4
#define HAND_NB_GROUPS 5
#define HISTO_INDEX_MAX 34
#define NB_PLAYERS 4
#define NO_TILE_INDEX 63

// histo_cell_t  is guaranted to be at most 3 bits
// histo_index_t is guaranted to be at most 7 bits
typedef unsigned char histo_cell_t;
typedef unsigned char histo_index_t;

enum group_type { PAIR, SEQUENCE, TRIPLET, QUAD };
enum player_type { PLAYER_HOST, PLAYER_AI, PLAYER_CLIENT };
enum riichi_state { NORIICHI, RIICHI, IPPATSU, DOUBLE_RIICHI, DOUBLE_IPPATSU };
enum table_pos { EAST, SOUTH, WEST, NORTH };

enum action {
	ACTION_RIICHI,
	ACTION_RON,
	ACTION_TSUMO,
	ACTION_DISCARD,
	ACTION_PASS,
	ACTION_CHII,
	ACTION_PON,
	ACTION_KAN
};

enum packet_type {
	PACKET_INIT,
	PACKET_DRAW,
	PACKET_INPUT,
	PACKET_TSUMO,
	PACKET_UPDATE
};

enum game_phase {
	PHASE_INIT,
	PHASE_DRAW,
	PHASE_GETINPUT,
	PHASE_TSUMO,
	PHASE_WAIT,
	PHASE_CLAIM
};

struct action_input {
	enum action action;
	histo_index_t tile;
};

#endif // _DEFINITIONS_H_
