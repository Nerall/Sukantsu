#define _POSIX_C_SOURCE 199309L
#include "net_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct net_client* init_client_from_host_and_port(const char* host, unsigned short port) 
{
	struct net_client *client = malloc(sizeof(struct net_client));
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
	wprintf(L"%s%s%s%hu\n", "Connected to ", client->host.address,":",client->port);
	return 1;
}

int send_to_server(struct net_client *client, const void* data, size_t size)
{
	if (sfTcpSocket_getRemoteAddress(client->socket).address == 0) {
		wprintf(L"%s\n", "Socket not connected");
		return 0;
	}
	if (sfTcpSocket_send(client->socket, data, size) != sfSocketDone) {
		wprintf(L"%s\n", "Fail while sending");
		return 0;
	}
	wprintf(L"%s\n\n", "Data sent");
	return 1;
}

int receive_from_server(struct net_client *client)
{
	if (sfTcpSocket_getRemoteAddress(client->socket).address == 0) {
		wprintf(L"%s\n", "Socket not connected");
		return 0;
	}
	char buf[256];
	size_t buflen = 256 * sizeof(char);
	size_t received = 1;
	while (received != 0) {
		sfSocketStatus s = sfTcpSocket_receive(client->socket, buf, buflen, &received);
		if (s == sfSocketError) {
			wprintf(L"%s\n", "Fail while receiving");
			return 0;
		}

		for (size_t i = 0; i < received; ++i) {
			wprintf(L"%c", buf[i]);
		}
	}
	wprintf(L"\n%s\n", "Data received");
	return 1;
}

void disconnect_from_server(struct net_client *client)
{
	sfIpAddress cur_address = sfTcpSocket_getRemoteAddress(client->socket);
	sfTcpSocket_disconnect(client->socket);
	wprintf(L"%s%s\n", "Disconnected from ", cur_address.address);
}

void client_buster(struct net_client *client)
{
	sfTcpSocket_destroy(client->socket);
	free(client);
}

void client_test() {

	struct net_client *client = init_client_from_host_and_port("perdu.com", 80);
	sfTime timeout = {microseconds: 60000000};
	client->timeout = timeout;

	if (!connect_to_server(client))
		return;

	const char *str = "GET http://perdu.com HTTP/1.0\n\r\n\r";
	size_t n = strlen(str);
	if (!send_to_server(client, str, n))
		return;

	if (!receive_from_server(client))
		return;

	disconnect_from_server(client);
	client_buster(client);
}
