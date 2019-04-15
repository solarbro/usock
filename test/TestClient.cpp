
// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <usock.h>
#define PORT 8080 
#define DEFAULT_BUFLEN 512
   
int main(int argc, char const *argv[]) 
{ 
    // struct sockaddr_in address; 
    usock_handle sock = 0; 
	int valread; 
	int iResult;
    // struct sockaddr_in serv_addr; 
    const char *hello = "Hello, Server!"; 
    char buffer[DEFAULT_BUFLEN] = {0}; 

	const char *ip = "127.0.0.1";

	if(argc >= 2)
	{
		ip = argv[1];
	}

	usock_initialize();

	usock_create_socket("Client socket", &sock);
	usock_configure(sock, USOCK_DOMAIN_IPV4, USOCK_SOCKTYPE_RELIABLE);
	
	/* connect */
	iResult = usock_connect(sock, ip, PORT);
	if(iResult != USOCK_OK)
	{
		printf("Connect failed\n");
		return iResult;
	}

	/* Send */
    iResult = usock_send(sock , hello , strlen(hello)); 
	if(iResult >= 0)
		printf("Hello message sent (%d bytes)\n", iResult); 
	else
	{
		printf("Send failed\n");
		usock_release();
		return 1;
	}

	/* Read */
    valread = usock_read( sock , buffer, DEFAULT_BUFLEN); 
	if(valread > 0)
	{
		printf("Message from server: %s\n", buffer); 
	}
	else
	{
		printf("No reply from server\n");
	}

	usock_release();

    return 0; 
} 
