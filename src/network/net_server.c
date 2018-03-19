#define _POSIX_C_SOURCE 199309L
#include "net_server.h"
#include "../debug.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Begin listening to the given port
void listen_net_server(struct net_server *server, unsigned port) {
	ASSERT_BACKTRACE(server);

	server->listener = sfTcpListener_create();
	printf("[SERVER] Listening to port %u\n", port);
	if (sfTcpListener_listen(server->listener, port, sfIpAddress_Any) !=
	    sfSocketDone) {
		server->listener = NULL;
		fprintf(stderr, "[ERROR][SERVER] Could not listen on port %u.\n", port);
		return;
	}

	sfTcpListener_setBlocking(server->listener, sfFalse);
	for (int i = 0; i < 4; ++i) {
		server->clients[i] = NULL;
	}
	server->nb_clients = 0;
}

// Stop listening
void stop_listen_net_server(struct net_server *server) {
	ASSERT_BACKTRACE(server);

	if (server->listener) {
		printf("[SERVER] Stoping listening\n");
		sfTcpListener_destroy(server->listener);
		server->listener = NULL;
	}
}

// Check if someone is trying to connect and add it to server->clients
// Do nothing if server->nb_clients >= 4
// Do not block (listener has been set to non-blocking in listen_net_server)
void check_new_connection_net_server(struct net_server *server) {
	ASSERT_BACKTRACE(server);

	if (server->nb_clients >= 4)
		return;

	sfTcpSocket *client;
	if (sfTcpListener_accept(server->listener, &client) == sfSocketDone) {
		printf("[SERVER] New client accepted\n");
		server->clients[server->nb_clients] = client;
		++server->nb_clients;
	}
}

// Clean the net_server
// Stop listening and destroy all clients
void clean_net_server(struct net_server *server) {
	stop_listen_net_server(server);

	for (int i = 0; i < 4; ++i) {
		if (server->clients[i]) {
			sfTcpSocket_destroy(server->clients[i]);
			server->clients[i] = NULL;
		}
	}

	server->nb_clients = 0;
}

void network_test() {
	printf("Net-Server test: (connect with nc for example)\n");
	const unsigned port = 7777;
	const time_t timeout = 5;
	const struct timespec delay = {tv_sec : 0, tv_nsec : 500 * 1000000};

	struct net_server server;
	listen_net_server(&server, port);
	time_t t1 = time(NULL);
	do {
		printf("%2lu s left\n", timeout - (time(NULL) - t1));
		check_new_connection_net_server(&server);
		nanosleep(&delay, NULL);
	} while (time(NULL) - t1 < timeout);
	clean_net_server(&server);
}
