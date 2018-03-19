#ifndef _NET_SERVER_H_
#define _NET_SERVER_H_

#include <SFML/Network.h>

struct net_server {
	sfTcpListener *listener;
	sfTcpSocket *clients[4];
	int nb_clients;
};

void listen_net_server(struct net_server *server, unsigned port);

void stop_listen_net_server(struct net_server *server);

void check_new_connection_net_server(struct net_server *server);

void clean_net_server(struct net_server *server);

void network_test();

#endif // _NET_SERVER_H_
