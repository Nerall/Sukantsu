#ifndef _NET_PACKET_
#define _NET_PACKET_

#include "../definitions.h"
#include "../core/histogram.h"

struct net_packet_init {
	enum packet_type packet_type; // Keep this at this place

	struct histogram *histogram;
};

struct net_packet_draw {
	enum packet_type packet_type; // Keep this at this place

	histo_index_t tile;
};

struct net_packet_input {
	enum packet_type packet_type; // Keep this at this place

	struct action_input input;
}

struct net_packet_update {
	enum packet_type packet_type; // Keep this at this place

	enum table_pos player_pos; // Player who made the action
	struct action_input input;
	char victory : 1;
};

#define MAX_PACKET_SIZE 255

struct net_packet {
	enum packet_type packet_type; // Keep this at this place

	char data[MAX_PACKET_SIZE];
};

#endif // _NET_PACKET_
