#include "net_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void client_test()
{
	sfTcpSocket* socket = sfTcpSocket_create();
	sfIpAddress host = {"208.97.177.124"};
	sfTime timeout = sfSeconds(60);
	if (sfTcpSocket_connect(socket, host, 80, timeout) != sfSocketDone)
	{
		printf("%s\n", "Fail while connecting");
		return;
	}
	const char* str = "GET http://perdu.com HTTP/1.0\n\r\n\r";
	size_t n = strlen(str);
	if (sfTcpSocket_send(socket, str, n) != sfSocketDone)
	{
		printf("%s\n", "Fail while sending");
		return;
	}
	char buf[256];
	size_t buflen = 256;
	size_t received = 1;
	while (received != 0){
		if(sfTcpSocket_receive(socket, buf, buflen, &received) != sfSocketDone)
		{
			printf("%s\n", "Fail while receiving");
			return;
		}
		printf("%s\n", buf);
	}

}
