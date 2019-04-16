
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <usock.h>
#include <usock.hpp>

#define PORT 8080 
#define DEFAULT_BUFLEN 512
   
int main(int argc, char const *argv[]) 
{ 
	const char *ip = "127.0.0.1";
	if(argc >= 2)
	{
		ip = argv[1];
	}

	usock::Instance usockInst;

    usock_handle sock = 0; 
	usock_create_socket("Client socket", &sock);
	usock_configure(
		sock, 
		USOCK_DOMAIN_IPV4, 
		USOCK_SOCKTYPE_RELIABLE,
		USOCK_OPTIONS_REUSE_ADDRESS);
	
	// Connect
	int iResult = usock_connect(sock, ip, PORT);
	if(iResult != USOCK_OK)
	{
		printf("Connect failed\n");
		return iResult;
	}

	// Send
    const char *hello = "Hello, Server!"; 
    iResult = usock_send(sock , hello , strlen(hello)); 
	if(iResult < 0)
	{
		printf("Send failed\n");
		return 1;
	}

	// Read
    char buffer[DEFAULT_BUFLEN] = {0}; 
    int valread = usock_read( sock , buffer, DEFAULT_BUFLEN); 
	if(valread > 0)
	{
		usock_release();
		// Ensure the string is correctly reversed
		size_t len = strlen(buffer);
		if(len != strlen(hello))
			return 3;

		for(size_t i = 0; i < len; ++i)
		{
			if(hello[i] != buffer[len - i - 1])
			{
				printf("String not correctly reversed by server\n");
				return 3;
			}
		}
	}
	else
	{
		printf("No reply from server\n");
		return 1;
	}

    return 0; 
} 
