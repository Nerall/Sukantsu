#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_

#include <SFML/Network.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct net_client {
	sfTcpSocket *socket;
	sfIpAddress host;
	unsigned short port;
	sfTime timeout;
};


struct net_client* init_client_from_host_and_port(const char* host, unsigned short port); 

int connect_to_server(struct net_client *client);

int send_to_server(struct net_client *client, const void* data, size_t size);

int receive_from_server(struct net_client *client, void* buf);

void disconnect_from_server(struct net_client *client);

void client_buster(struct net_client *client);

void client_test();

#endif // _NET_CLIENT_H_
