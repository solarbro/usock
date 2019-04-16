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

#include <usock.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

usock_err translate_win32_error(int error)
{
	switch (error)
	{
	case WSANOTINITIALISED:
		return USOCK_ERROR_NOT_INITIALIZED;
	case WSAENETDOWN:
		return USOCK_ERROR_NETWORK_DOWN;
	case WSAEINVAL:
		return USOCK_ERROR_INVALID_ARG;
	case WSAEPROTONOSUPPORT:
		return USOCK_ERROR_PROTOCOL_NOT_SUPPORTED;
	default:
		return USOCK_ERROR_INTERNAL;
	}
}

int usock_initialize()
{
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
		return USOCK_ERROR_INIT_FAILED;

	return USOCK_OK;
}

void usock_release()
{
	WSACleanup();
}

int usock_open_socket(usock_socket * outSocket, usock_domain domain, usock_socket_type type)
{
	int family;
	switch(domain)
	{
	case USOCK_DOMAIN_IPV4:
		family = AF_INET;
		break;
	case USOCK_DOMAIN_IPV6:
		family = AF_INET6;
		break;
	default:
		family = AF_UNSPEC;
		break;
	}

	int stype;
	int protocol;
	switch (type)
	{
	case USOCK_SOCKTYPE_FAST:
		stype = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
		break;
	case USOCK_SOCKTYPE_RELIABLE:
		stype = SOCK_STREAM;
		protocol = IPPROTO_TCP;
		break;
	default:
		stype = 0;    //Not sure this is a good idea!!
		protocol = 0;
		break;
	}

	outSocket->hsock.winsock = socket(family, stype, protocol);
	if(outSocket->hsock.winsock == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		return translate_win32_error(err);
	}

	// Copy reserved values
	memcpy(outSocket->reserved, &family, sizeof(unsigned));
	memcpy(outSocket->reserved + 1, &protocol, sizeof(unsigned));
	return USOCK_OK;
}

int usock_get_addr_info(const char * node, const char * service, const usock_addr_info * pHints, usock_addr_info ** ppOutResults)
{
	struct addrinfo hints, *result = NULL;
	int error;
	switch(pHints->protocol)
	{
	case USOCK_SOCKTYPE_FAST:
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;
		break;
	case USOCK_SOCKTYPE_RELIABLE:
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		break;
	default:
		hints.ai_socktype = 0;
		hints.ai_protocol = 0;
		break;
	}
	switch(pHints->family)
	{
	case USOCK_DOMAIN_IPV4:
		hints.ai_family = AF_INET;
		break;
	case USOCK_DOMAIN_IPV6:
		hints.ai_family = AF_INET6;
		break;
	default:
		hints.ai_family = AF_UNSPEC;
		break;
	}

	error = getaddrinfo(node, service, &hints, &result);
	if(error != 0)
	{
		*ppOutResults = NULL;
		return translate_win32_error(error);
	}

	*ppOutResults = (usock_addr_info*)malloc(sizeof(usock_addr_info));

	return 0;
}

int usock_bind(const usock_socket *socket, unsigned port)
{
	struct sockaddr_in address;
	int err;
	address.sin_family = (ADDRESS_FAMILY) socket->reserved[0];
	address.sin_port = htons(port);
	address.sin_addr.S_un.S_addr = INADDR_ANY;

	err = bind(socket->hsock.winsock, (struct sockaddr*)&address, sizeof(address));
	return translate_win32_error(err);
}

int usock_listen(const usock_socket *socket, int backlog)
{
	int err;
	err = listen(socket->hsock.winsock, backlog);
	return translate_win32_error(err);
}

int usock_accept(const usock_socket *socket, usock_address * addr, unsigned * addr_len)
{
	int err;
	int nameLen;
	err = accept(socket->hsock.winsock, (struct sockaddr*)addr, &nameLen);
	*addr_len = (unsigned)nameLen;
	return translate_win32_error(err);
}

int usock_connect(const usock_socket *socket, const char *ip_address, unsigned port)
{
	int err;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(addr.sin_family, ip_address, &addr.sin_addr);
	err = connect(socket->hsock.winsock, &addr, sizeof(addr));
	return translate_win32_error(err);
}

int usock_close_socket(usock_socket *socket)
{
	int err;
	err = closesocket(socket->hsock.winsock);
	return translate_win32_error(err);
}

#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#   error "Unknown Apple platform"
#endif
#elif __linux__
// linux
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

#define MAX_SOCKET_NAME_LEN 32

struct SockInfo
{
	/* All required info about the socket. */
	int socket;
	char name[MAX_SOCKET_NAME_LEN];

	struct sockaddr_in info;
	int protocol;
	unsigned sockopt;

	struct SockInfo *prev, *next;
};

void initSockInfo(struct SockInfo *node, const char *name)
{
	memset(node, 0, sizeof(struct SockInfo));
	strncpy(node->name, name, MAX_SOCKET_NAME_LEN);
}

struct SockInfo *g_head = NULL;
struct SockInfo *g_tail = NULL;

void printNodeList()
{
	struct SockInfo *node = g_head;
	while(node)
	{
		// printf("%s", node->name);
		if(node->next)
		{
			// printf("->");
		}
		node = node->next;
	}
	// printf("\n");
}

int usock_initialize()
{
	return USOCK_OK;
}

void usock_release()
{
	struct SockInfo *si = g_head;

	/* Free all allocated socket nodes */
	while(si)
	{
		struct SockInfo *next = si->next;
		usock_close_socket(si);
		free(si);
		si = next;
	}
}

int usock_create_socket(const char *name, usock_handle *pOutSocket)
{
	struct SockInfo *node = (struct SockInfo *)malloc(sizeof(struct SockInfo));
	if(!node)
	{
		*pOutSocket = NULL;
		return USOCK_ERROR_OUT_OF_MEMORY;
	}

	initSockInfo(node, name);

	if(!g_tail)
	{
		g_head = g_tail = node;
	}
	else
	{
		g_tail->next = node;
		node->prev = g_tail;
		g_tail = node;
	}
	
	*pOutSocket = (void*)node;

	/* Debug code */
	printNodeList();

	return USOCK_OK;
}

void usock_configure(usock_handle hsock, usock_domain domain, usock_socket_type type, uint32_t flags)
{
	struct SockInfo *info = (struct SockInfo*)hsock;
	switch(domain)
	{
	case USOCK_DOMAIN_IPV4:
		info->info.sin_family = AF_INET;
		break;
	case USOCK_DOMAIN_IPV6:
		info->info.sin_family = AF_INET6;
		break;
	default:
		info->info.sin_family = AF_UNSPEC;
		break;
	}

	switch (type)
	{
	case USOCK_SOCKTYPE_FAST:
		info->protocol = SOCK_DGRAM;
		break;
	case USOCK_SOCKTYPE_RELIABLE:
		info->protocol = SOCK_STREAM;
		break;
	}

	info->sockopt = flags;
}

int usock_bind(usock_handle hsocket, unsigned port)
{
	struct SockInfo *node = (struct SockInfo*)hsocket;
	int ret;
	int val;

	node->socket = socket(node->info.sin_family, node->protocol, 0);

	if(node->socket == 0)
	{
		/* Error handling */
		return USOCK_ERROR_INIT_FAILED;
	}

	/* Set socket options */
	val = node->sockopt & USOCK_OPTIONS_REUSE_ADDRESS;
	ret = setsockopt(node->socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	val = node->sockopt & USOCK_OPTIONS_REUSE_PORT;
	ret = setsockopt(node->socket, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));

	/* Bind the socket */
	node->info.sin_addr.s_addr = INADDR_ANY;
	node->info.sin_port = htons(port);

	ret = bind(node->socket, (struct sockaddr *)&node->info, sizeof(node->info));
	return ret < 0 ? USOCK_ERROR_INIT_FAILED : USOCK_OK;
}

int usock_listen(usock_handle hsock, int backlog)
{
	int ret;
	struct SockInfo *node = (struct SockInfo *)hsock;
	if(!node->socket)
		return USOCK_ERROR_NOT_INITIALIZED;

	ret = listen(node->socket, backlog);
	if(ret < 0)
		return USOCK_ERROR_INTERNAL;

	return USOCK_OK;
}

int usock_accept(usock_handle hsock, usock_handle *pOutSock)
{
	int newSock;
	struct sockaddr_in address;
	socklen_t len;
	struct SockInfo *node = (struct SockInfo *)hsock;
	struct SockInfo **outNode = (struct SockInfo **)pOutSock;

	newSock = accept(node->socket, (struct sockaddr *)&address, &len);
	if(newSock < 0)
	{
		*pOutSock = NULL;
		return USOCK_ERROR_INTERNAL;
	}

	usock_create_socket("client socket", pOutSock);

	/* Cache the returned socket info */
	(*outNode)->socket = newSock;
	memcpy(&(*outNode)->info, &address, len);

	return USOCK_OK;
}

int usock_connect( usock_handle hsock, const char *ip_address, unsigned short port )
{
	int ret;
	struct SockInfo *node = (struct SockInfo *)hsock;

	/* open socket */
	node->socket = socket(node->info.sin_family, node->protocol, 0);
	if(node->socket < 0)
	{
		return USOCK_ERROR_INIT_FAILED;
	}

	node->info.sin_port = htons(port);
	if(inet_pton(node->info.sin_family, ip_address, &node->info.sin_addr) <= 0)
	{
		/* address parse failed */
		return USOCK_ERROR_INVALID_ARG;
	}

	ret = connect(node->socket, (struct sockaddr *)&node->info, sizeof(node->info));
	if(ret < 0)
	{
		/* connect failed */
		return USOCK_ERROR_INTERNAL;
	}

	return USOCK_OK;
}

int usock_read(usock_handle hsock, void *pOutBuffer, unsigned long long buflen)
{
	struct SockInfo *node = (struct SockInfo *)hsock;
	return read(node->socket, pOutBuffer, buflen);
}

int usock_send(usock_handle hsock, const void *buffer, unsigned long long buflen)
{
	struct SockInfo *node = (struct SockInfo *)hsock;
	return send(node->socket, buffer, buflen, 0);
}

void usock_close_socket(usock_handle hsock)
{
	/* Close the connection */
	struct SockInfo *node = (struct SockInfo *)hsock;
	if(node->socket)
		close(node->socket);
	node->socket = 0;
}

void usock_free_socket(usock_handle hsock)
{
	struct SockInfo *node = (struct SockInfo *)hsock;

	/* Detach the node from the list */
	if(node->prev)
	{
		node->prev->next = node->next;
	}
	if(node->next)
	{
		node->next->prev = node->prev;
	}

	if(node == g_head && node == g_tail)
	{
		/* Freeing the last node */
		g_head = g_tail = NULL;
	}
	else if(node == g_tail)
	{
		/* Freeing the tail node. 
		*  Update tail pointer.
		*/
		g_tail = g_tail->prev;
	}
	else if(node == g_head)
	{
		/* Freeing the head node. 
		*  Update head pointer.
		*/
		g_head = g_head->next;
	}
	
	/* Free allocated node */
	free(node);

	/* Debug code */
	printNodeList();
}

#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#   error "Unknown compiler"
#endif


