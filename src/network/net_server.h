#ifndef _NET_SERVER_H_
#define _NET_SERVER_H_

#include <SFML/Network.h>
#include <time.h>

struct net_server;

int listen_net_server(struct net_server *server, unsigned short port_min,
                      unsigned short port_max);

void stop_listen_net_server(struct net_server *server);

int check_new_connection_net_server(struct net_server *server);

void clean_net_server(struct net_server *server);

int send_data_to_client(struct net_server *server, int iclient, void *data,
                        size_t data_size);

int receive_data_from_client(struct net_server *server, int iclient, void *data,
                             size_t data_size, sfTime timeout);

#endif // _NET_SERVER_H_
