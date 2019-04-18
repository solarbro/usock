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
#include <string.h> 
#include <string>
#include <unistd.h> 
#include <stdlib.h> 

#include <usock.hpp>

#define DEFAULT_BUFLEN 512
#define PORT 8081

int main(int argc, const char *argv[])
{
	usock::instance usockInst;

	const char *ip_address = "127.0.0.1";

	if(argc > 1)
		ip_address = argv[1];

    // create datagram socket 
	usock_handle_t sockfd;
	usock_create_socket("", &sockfd);
	usock_configure(sockfd, USOCK_DOMAIN_IPV4, USOCK_SOCKTYPE_FAST, USOCK_OPTIONS_DEFAULT);

    // connect to server 
	int ret = usock_connect(sockfd, ip_address, PORT);
	if(ret != USOCK_OK)
	{
		printf("Connect failed\n");
		return 1;
	}
  
    // request to send datagram 
    // no need to specify server address in sendto 
    // connect stores the peers IP and port 
    std::string message = "Hello Server"; 
	usock_send_to(sockfd, message.c_str(), message.length(), 0, NULL);
      
    // waiting for response 
	char buffer[100]; 
	int n = usock_recv_from(sockfd, buffer, sizeof(buffer), 0, NULL);

	//Ensure string is correctly reversed
	if((size_t)n != message.length())
		return 2;

	for(size_t i = 0; i < message.length(); ++i)
	{
		if(buffer[i] != message[message.length() - i - 1])
			return 2;
	}
  
	///////////////////////////
	return 0;
}