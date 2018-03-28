#ifndef _NET_CLIENT_S_H_
#define _NET_CLIENT_S_H_

#include <SFML/Network.h>

struct net_client {
	sfTcpSocket *socket;
};

#endif // _NET_CLIENT_S_H_
