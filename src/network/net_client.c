#include "net_client.h"
#include "../debug.h"
#include "net_client_s.h"
#include <stdio.h>

int connect_to_server(struct net_client *client, const char *host,
                      unsigned short port) {
	ASSERT_BACKTRACE(client);

	const sfTime timeout = {microseconds : 30000000};

	client->socket = sfTcpSocket_create();
	sfIpAddress address = sfIpAddress_fromString(host);

	if (sfTcpSocket_connect(client->socket, address, port, timeout) !=
	    sfSocketDone) {
		char temp[16];
		sfIpAddress_toString(address, temp);
		fprintf(stderr, "Fail while connecting to %s:%hu\n", temp, port);
		return 0;
	}

	fprintf(stderr, "Connected to %s:%hu\n", host, port);
	return 1;
}

int send_to_server(struct net_client *client, const void *data, size_t size) {
	ASSERT_BACKTRACE(client);
	ASSERT_BACKTRACE(data);
	ASSERT_BACKTRACE(size > 0);

	if (sfTcpSocket_getRemoteAddress(client->socket).address == 0) {
		fprintf(stderr, "Socket not connected\n");
		return 0;
	}

	if (sfTcpSocket_send(client->socket, data, size) != sfSocketDone) {
		fprintf(stderr, "Fail while sending\n");
		return 0;
	}

	fprintf(stderr, "Data sent\n");
	return 1;
}

int receive_from_server(struct net_client *client, void *buf, size_t buf_size) {
	ASSERT_BACKTRACE(client);
	ASSERT_BACKTRACE(buf);
	ASSERT_BACKTRACE(buf_size > 0);

	if (sfTcpSocket_getRemoteAddress(client->socket).address == 0) {
		fprintf(stderr, "Socket not connected\n");
		return 0;
	}

	size_t received;
	sfSocketStatus s;
	do {
		s = sfTcpSocket_receive(client->socket, buf, buf_size, &received);
		if (s == sfSocketError) {
			fprintf(stderr, "Fail while receiving\n");
			return 0;
		}
	} while (received > 0);

	fprintf(stderr, "Data received\n");
	return 1;
}

void disconnect_from_server(struct net_client *client) {
	ASSERT_BACKTRACE(client);

	sfIpAddress cur_address = sfTcpSocket_getRemoteAddress(client->socket);
	sfTcpSocket_disconnect(client->socket);
	fprintf(stderr, "Disconnected from %s\n", cur_address.address);

	sfTcpSocket_destroy(client->socket);
}

/*
void client_test() {
    struct net_client *client = init_client_from_host_and_port("perdu.com", 80);
    sfTime timeout = {microseconds : 60000000};
    client->timeout = timeout;

    if (!connect_to_server(client))
        return;

    const char *str = "GET http://perdu.com HTTP/1.0\n\r\n\r";
    size_t n = strlen(str);
    if (!send_to_server(client, str, n))
        return;
    char buf[256];
    if (!receive_from_server(client, buf))
        return;

    disconnect_from_server(client);
    client_buster(client);
}
*/
