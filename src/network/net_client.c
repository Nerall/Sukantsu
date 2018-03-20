#define _POSIX_C_SOURCE 199309L
#include "net_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

sfTcpSocket* init_client_from_host_and_port(const char* host, unsigned short port) 
{
	struct net_client *client;
	client->socket = sfTcpSocket_create();
	client->host = sfIpAddress_fromString(host);
	client->port = port;
	return client;
}

int connect_to_server(struct net_client *client)
{
	if (sfTcpSocket_connect(client->socket, client->host, client->port, client->timeout) != sfSocketDone) {
		wprintf(L"%s\n", "Fail while connecting");
		return 0;
	}
	wprintf(L"%s%s%hu\n", "Connected to", client->host.address, client->port);
	return 1;
}

int disconnect_from_server(struct net_client *client)
{
	sfIpAddress cur_address = sfTcpSocket_getRemoteAddress(client->socket);
	sfTcpSocket_disconnect(client->socket);
	wprintf(L"%s%s\n", "Disconnected from", cur_address.address);
}

void client_test() {
	sfTcpSocket *socket = sfTcpSocket_create();
	sfIpAddress host = sfIpAddress_fromString("perdu.com");
	sfTime timeout = {microseconds: 60000000};

	if (sfTcpSocket_connect(socket, host, 80, timeout) != sfSocketDone) {
		wprintf(L"%s\n", "Fail while connecting");
		return;
	}

	const char *str = "GET http://perdu.com HTTP/1.0\n\r\n\r";
	size_t n = strlen(str);
	if (sfTcpSocket_send(socket, str, n) != sfSocketDone) {
		wprintf(L"%s\n", "Fail while sending");
		return;
	}

	char buf[256];
	size_t buflen = 256 * sizeof(char);
	size_t received = 1;
	while (received != 0) {
		sfSocketStatus s = sfTcpSocket_receive(socket, buf, buflen, &received);
		if (s == sfSocketError) {
			wprintf(L"%s\n", "Fail while receiving");
			return;
		}

		for (size_t i = 0; i < received; ++i) {
			wprintf(L"%c", buf[i]);
		}
	}

	sfTcpSocket_destroy(socket);
}
