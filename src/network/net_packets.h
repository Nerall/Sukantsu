#ifndef _NET_PACKET_
#define _NET_PACKET_

#include "../core/histogram_s.h"
#include "../definitions.h"

struct net_packet_init {
	enum packet_type packet_type; // Keep this at this place

	enum table_pos player_pos;
	struct histogram histo;
};

struct net_packet_draw {
	enum packet_type packet_type; // Keep this at this place

	histo_index_t tile;
	unsigned char nb_wall_tiles;
};

struct net_packet_input {
	enum packet_type packet_type; // Keep this at this place

	struct action_input input;
};

struct net_packet_update {
	enum packet_type packet_type; // Keep this at this place

	enum table_pos player_pos; // Player who made the action
	struct action_input input;
	char victory : 1;
};

struct net_packet_tsumo {
	enum packet_type packet_type; // Keep this at this place

	enum table_pos player_pos; // Player who won
	struct histogram histo;    // Victory histogram of the player
};

#define MAX_PACKET_SIZE                                                        \
	(sizeof(struct net_packet_update) > sizeof(struct net_packet_init)         \
	     ? sizeof(struct net_packet_update)                                    \
	     : sizeof(struct net_packet_init))

struct net_packet {
	enum packet_type packet_type; // Keep this at this place

	char data[MAX_PACKET_SIZE - sizeof(enum packet_type)];
};

#endif // _NET_PACKET_
