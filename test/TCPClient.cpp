/******************************************************************************
Copyright (c) 2019 Sagnik Chowdhury

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

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
