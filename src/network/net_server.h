#ifndef _NET_SERVER_H_
#define _NET_SERVER_H_

#include <SFML/Network.h>

struct net_server {
	sfTcpListener *listener;
	sfTcpSocket *clients[4];
};

void network_test();

#endif // _NET_SERVER_H_
