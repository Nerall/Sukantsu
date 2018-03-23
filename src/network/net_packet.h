#ifndef _NET_PACKET_
#define _NET_PACKET_

#include "../definitions.h"
#include "../core/histogram.h"

struct net_packet_init {
	enum packet_type packet_type;
	struct histogram *histogram;
};

struct net_packet_draw {
	enum packet_type packet_type;
	// TODO: Complete the structure
};

struct net_packet_update {
	enum packet_type packet_type;
	// TODO: Complete the structure
};

#define MAX_PACKET_SIZE sizeof(net_packet_update)

struct net_packet {
	enum packet_type packet_type;
	char data[MAX_PACKET_SIZE];
};

#endif // _NET_PACKET_
