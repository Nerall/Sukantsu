#define _POSIX_C_SOURCE 199309L
#include "net_server.h"
#include "../debug.h"
#include "net_server_s.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

// Begin listening to the given port
// Return 0 if no error occured
int listen_net_server(struct net_server *server, unsigned short port_min,
                      unsigned short port_max) {
	ASSERT_BACKTRACE(server);

	server->listener = sfTcpListener_create();
	for (unsigned short port = port_min; port < port_max; port += 100) {
		wprintf(L"[SERVER] Trying listening to port %u\n", port);
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

	wprintf(L"[ERROR][SERVER] Could not connect to any port in [%u, %u[",
	        port_min, port_max);
	return 1;
}

// Stop listening
void stop_listen_net_server(struct net_server *server) {
	ASSERT_BACKTRACE(server);

	if (server->listener) {
		sfTcpListener_destroy(server->listener);
		wprintf(L"[SERVER] Stoped listening\n");
		server->listener = NULL;
	}
}

// Check if someone is trying to connect and add it to server->clients
// Do nothing if server->nb_clients >= 4
// Do not block (listener has been set to non-blocking in listen_net_server)
// Return 1 if a player has connected
int check_new_connection_net_server(struct net_server *server) {
	ASSERT_BACKTRACE(server);

	if (server->nb_clients >= 4)
		return 0;

	sfTcpSocket *client;
	if (sfTcpListener_accept(server->listener, &client) == sfSocketDone) {
		wprintf(L"[SERVER] New client accepted\n");
		server->clients[server->nb_clients] = client;
		++server->nb_clients;
		return 1;
	}
	return 0;
}

// Clean the net_server
// Stop listening and destroy all clients
void clean_net_server(struct net_server *server) {
	ASSERT_BACKTRACE(server);

	stop_listen_net_server(server);

	for (int i = 0; i < 4; ++i) {
		if (server->clients[i]) {
			sfTcpSocket_destroy(server->clients[i]);
			server->clients[i] = NULL;
		}
	}

	server->nb_clients = 0;
}

// Send data to the client
// If operation takes more than timeout or an error occured, return 1
// If data has been sent without problem, return 0
// The timeout argument is expressed in seconds
int send_data_to_client(struct net_server *server, int iclient, void *data,
                        size_t data_size, time_t timeout_s) {
	ASSERT_BACKTRACE(server);
	ASSERT_BACKTRACE(server->clients[iclient]);

	const struct timespec delay = {tv_sec : 0, tv_nsec : 100 * 1000000};
	sfTcpSocket *client = server->clients[iclient];
	sfSocketStatus status;
	time_t t1 = time(NULL);

	do {
		size_t sent;
		switch (sfTcpSocket_sendPartial(client, data, data_size, &sent)) {
			case sfSocketDone:
				return 0;

			case sfSocketPartial: {
				// Shift based on the number of byte sent
				data += sent;
				data_size -= sent;
				break;
			}

			default:
				return 1;
		}

		nanosleep(&delay, NULL);
	} while (time(NULL) - t1 < timeout_s);

	status = sfTcpSocket_send(client, data, data_size);

	return status != sfSocketDone;
}

// Receive data from the client
// If operation takes more than timeout or an error occured, return 1
// If data has been received without problem, return 0
// The timeout argument is expressed in seconds
// The data argument may still be modified when the function returns 1
int receive_data_from_client(struct net_server *server, int iclient, void *data,
                             size_t data_size, time_t timeout_s) {
	ASSERT_BACKTRACE(server);
	ASSERT_BACKTRACE(server->clients[iclient]);

	const struct timespec delay = {tv_sec : 0, tv_nsec : 50 * 1000000};
	sfTcpSocket *client = server->clients[iclient];
	sfSocketStatus status;
	time_t t1 = time(NULL);
	size_t obt;

	do {
		switch (sfTcpSocket_receive(client, data, data_size, &obt)) {
			case sfSocketDone:
				return 0;

			case sfSocketPartial: {
				// Shift based on the number of byte received
				data += obt;
				data_size -= obt;
				break;
			}

			default:
				return 1;
		}

		nanosleep(&delay, NULL);
	} while (time(NULL) - t1 < timeout_s);

	status = sfTcpSocket_receive(client, data, data_size, &obt);

	return status != sfSocketDone;
}
