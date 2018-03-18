#include "network.h"
#include <stdio.h>
#include <string.h>
#include <SFML/Network.h>

void network_test() {
	printf("This is the server (use nc for example to connect)\n");
	const unsigned port = 53000;

	const char data[] = {82,  111, 115, 101, 115, 32,  97,  114, 101, 32,
	                     114, 101, 100, 44,  10,  86,  105, 111, 108, 101,
	                     116, 115, 32,  97,  114, 101, 32,  98,  108, 117,
	                     101, 46,  10,  79,  109, 97,  101, 32,  119, 97,
	                     32,  109, 111, 117, 10,  83,  104, 105, 110, 100,
	                     101, 105, 114, 117, 33,  10,  0};

	const int len = strlen(data);

	sfTcpListener *listener = sfTcpListener_create();
	printf("Listening on port %u...\n", port);
	if (sfTcpListener_listen(listener, port, sfIpAddress_Any) != sfSocketDone) {
		printf("Error on listen\n");
		return;
	}
	printf("Listen done.\n");

	sfTcpSocket *client;
	printf("Accepting on port %u...\n", port);
	if (sfTcpListener_accept(listener, &client) != sfSocketDone) {
		printf("Error on accept\n");
		return;
	}
	printf("Accept done.\n");

	if (sfTcpSocket_send(client, data, len) != sfSocketDone) {
		printf("Error on send\n");
		return;
	}

	sfTcpSocket_destroy(client);
	sfTcpListener_destroy(listener);
}
