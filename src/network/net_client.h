#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include <SFML/Network.h>

struct net_client {
	sfTcpSocket *socket;
	sfIpAddress host;
	unsigned short port;
};

int connect_to_server(struct net_client *client, const char *host,
                      unsigned short port);

int send_to_server(struct net_client *client, const void *data, size_t size);

int receive_from_server(struct net_client *client, void *buf, size_t buf_size);

void disconnect_from_server(struct net_client *client);

#endif // _NET_CLIENT_H_
