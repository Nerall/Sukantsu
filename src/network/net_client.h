#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include <SFML/Network.h>

struct net_client {
	sfTcpSocket *socket;
	sfIpAddress host;
	unsigned short port;
	sfTime timeout;
};

void client_test();

#endif // _NET_CLIENT_H_
