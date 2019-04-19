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

#include <stdlib.h>
#include <stdio.h>
#include <usock.h>
#include <usock.hpp>
#include <string.h>

#define DEFAULT_BUFLEN 512
#define PORT 8080

void reverseStr(char *str);

int main(int argc, const char *argv[])
{
	usock::instance usockInst;

	usock_handle_t ListenSocket = nullptr;
	usock_create_socket("Listen socket", &ListenSocket);
	usock_configure(
		ListenSocket, 
		USOCK_DOMAIN_IPV4, 
		USOCK_SOCKTYPE_RELIABLE,
		USOCK_OPTIONS_REUSE_ADDRESS);

	// Create a SOCKET for connecting to server
	int iResult = usock_bind(ListenSocket, PORT);
	if(iResult != USOCK_OK)
	{
		printf("Failed to bind socket.\n");
		return iResult;
	}

	iResult = usock_listen(ListenSocket, 3);
	if(iResult != USOCK_OK)
	{
		printf("Failed to listen \n");
		return iResult;
	}

	usock_handle_t ClientSocket = nullptr;
	iResult = usock_accept(ListenSocket, &ClientSocket);
	if(iResult != USOCK_OK)
	{
		printf("Failed to accept socket \n");
		return iResult;
	}

	char buffer[DEFAULT_BUFLEN] = {};
	usock_ssize_t valread = usock_recv(ClientSocket, buffer, DEFAULT_BUFLEN);

	if(valread > 0)
	{
		reverseStr(buffer);
		usock_send(ClientSocket, buffer, strlen(buffer));
	}

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