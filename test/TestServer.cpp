#include <stdlib.h>
#include <stdio.h>
#include <usock.h>
#include <string.h>

#define DEFAULT_BUFLEN 512
#define PORT 8080

void reverseStr(char *str);

int main(int argc, const char *argv[])
{
	int iResult;

	usock_handle ListenSocket = nullptr;
	usock_handle ClientSocket = nullptr;

	int valread;
	char buffer[DEFAULT_BUFLEN];

	// Initialize Winsock
	iResult = usock_initialize();
	if (iResult != USOCK_OK) 
	{
		printf("usock_initialize failed with error: %d\n", iResult);
		return iResult;
	}

	usock_create_socket("Listen socket", &ListenSocket);
	usock_configure(ListenSocket, USOCK_DOMAIN_IPV4, USOCK_SOCKTYPE_RELIABLE);

	// Create a SOCKET for connecting to server
	iResult = usock_bind(ListenSocket, PORT);
	if(iResult != USOCK_OK)
	{
		printf("Failed to bind socket.\n");
		usock_release();
		return iResult;
	}

	iResult = usock_listen(ListenSocket, 3);
	if(iResult != USOCK_OK)
	{
		printf("Failed to listen \n");
		usock_close_socket(ListenSocket);
		usock_release();
		return iResult;
	}

	iResult = usock_accept(ListenSocket, &ClientSocket);
	if(iResult != USOCK_OK)
	{
		printf("Failed to accept socket \n");
		usock_close_socket(ListenSocket);
		usock_release();
		return iResult;
	}

	memset(buffer, 0, sizeof(buffer));
	printf("Start client read\n");
	valread = usock_read(ClientSocket, buffer, DEFAULT_BUFLEN);
	printf("Finish client read (%d bytes)\n", valread);

	if(valread > 0)
	{
		printf("Message from client: %s\n", buffer);
		reverseStr(buffer);
		printf("Message from server: %s\n", buffer);
		usock_send(ClientSocket, buffer, strlen(buffer));
	}

	// shutdown the connection since we're done
	usock_close_socket(ClientSocket);

	// cleanup
	usock_release();

	printf("Exiting server\n");

	return 0;
}

void reverseStr(char *str)
{
	size_t l = strlen(str);
	size_t m = l / 2;

	for(size_t i = 0; i < m; ++i)
	{
		char c = str[i];
		str[i] = str[l - i - 1];
		str[l - i - 1] = c;
	}
}