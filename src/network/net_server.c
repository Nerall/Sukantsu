#define _POSIX_C_SOURCE 199309L
#include "net_server.h"
#include "../debug.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Begin listening to the given port
// Return 0 if no error occured
int listen_net_server(struct net_server *server, unsigned short port_min,
                      unsigned short port_max) {
	ASSERT_BACKTRACE(server);

	server->listener = sfTcpListener_create();
	for (unsigned short port = port_min; port < port_max; ++port) {
		printf("[SERVER] Trying listening to port %u\n", port);
		if (sfTcpListener_listen(server->listener, port, sfIpAddress_Any) ==
		    sfSocketDone) {
			sfTcpListener_setBlocking(server->listener, sfFalse);
			for (int i = 0; i < 4; ++i) {
				server->clients[i] = NULL;
			}
			server->nb_clients = 0;
			return 0;
		}
	}

	fprintf(stderr, "[ERROR][SERVER] Could not connect to any port in [%u, %u[",
	        port_min, port_max);
	return 1;
}

// Stop listening
void stop_listen_net_server(struct net_server *server) {
	ASSERT_BACKTRACE(server);

	if (server->listener) {
		sfTcpListener_destroy(server->listener);
		printf("[SERVER] Stoped listening\n");
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
	const unsigned short port_min = 5000, port_max = 6000;
	const time_t timeout = 5;
	const struct timespec delay = {tv_sec : 0, tv_nsec : 500 * 1000000};
	const struct timespec delay2 = {tv_sec : 0, tv_nsec : 50 * 1000000};

	char *message = "Tell me something\n> ";
	char *message2 = "I've received that!\n";

	struct net_server server;
	if (listen_net_server(&server, port_min, port_max)) {
		return;
	}

	time_t t1 = time(NULL);
	do {
		printf("%2lu s left\n", timeout - (time(NULL) - t1));
		check_new_connection_net_server(&server);
		nanosleep(&delay, NULL);
	} while (time(NULL) - t1 < timeout);

	for (int i = 0; i < 4; ++i) {
		sfTcpSocket *client = server.clients[i];
		if (client) {
			sfSocketStatus status;
			status = sfTcpSocket_send(client, message, strlen(message));
			if (status != sfSocketDone) {
				fprintf(stderr, "[ERROR][SERVER] Send error\n");
			}

			char *buffer[1024];
			size_t n;
			status =
			    sfTcpSocket_receive(client, buffer, 1024 * sizeof(char), &n);
			if (status != sfSocketDone) {
				fprintf(stderr, "[ERROR][SERVER] Receive error\n");
			}

			status = sfTcpSocket_send(client, message2, strlen(message2));
			if (status != sfSocketDone) {
				fprintf(stderr, "[ERROR][SERVER] Send error\n");
			}

			status = sfTcpSocket_send(client, buffer, n);
			if (status != sfSocketDone) {
				fprintf(stderr, "[ERROR][SERVER] Send error\n");
			}
		}
	}

	clean_net_server(&server);
}
