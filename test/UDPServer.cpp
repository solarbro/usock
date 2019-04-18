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
#include <string.h>
#include <string>
#include <usock.hpp>

#define DEFAULT_BUFLEN 512
#define PORT 8081

int main(int argc, const char *argv[])
{
	usock::instance usockInst;

    // Create a UDP Socket 
	usock_handle_t listenfd;
	usock_create_socket("listen socket", &listenfd);
	usock_configure(
		listenfd, 
		USOCK_DOMAIN_IPV4, 
		USOCK_SOCKTYPE_FAST, 
		USOCK_OPTIONS_DEFAULT
	);

    // bind server address to socket descriptor 
	int ret = usock_bind(listenfd, PORT);
	if(ret != USOCK_OK)
	{
		printf("Bind failed\n");
		return 1;
	}
   
    //receive the datagram 
	usock_handle_t client;
	char buffer[100]; 
	int n = usock_recv_from(listenfd, buffer, sizeof(buffer), 0, &client);

    buffer[n] = '\0'; 

	//Reverse string
	std::string message;
	message.reserve(n);
	for(int i = 0; i < n; ++i)
	{
		message.push_back(buffer[n - i - 1]);
	}
           
    // send the response 
	usock_send_to(listenfd, message.c_str(), message.length(), 0, client);

	return 0;
}