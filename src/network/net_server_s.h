#ifndef _NET_SERVER_S_H_
#define _NET_SERVER_S_H_

#include <SFML/Network.h>

struct net_server {
	sfTcpListener *listener;
	sfSocketSelector *selector;
	sfTcpSocket *clients[4];
	int nb_clients;
};

#endif // _NET_SERVER_S_H_
