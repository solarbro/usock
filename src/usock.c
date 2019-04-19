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
#include <string.h>
#include <stdio.h>

/***************************************/
/*            COMMON CODE              */
/***************************************/
#define MAX_SOCKET_NAME_LEN 32

typedef struct SockInfoNode
{
	char name[MAX_SOCKET_NAME_LEN];
	struct SockInfoNode *prev, *next;
	size_t blockSize;
} SockInfoNode;

/***************************************/
/*          Socket node list           */
struct SockInfoNode *g_head  = NULL;
struct SockInfoNode *g_tail  = NULL;
/* TODO: Add mutex */

/***************************************/
/*        Allocator functions          */
usock_palloc_t g_palloc      = NULL;
usock_pfree_t  g_pfree       = NULL;

/***************************************/
/* Check if the library is initialized */
int            g_initialized = 0;

usock_err_t usock_set_custom_allocator(const usock_allocator *allocator)
{
	if(g_initialized)
		return USOCK_ERROR_ALREADY_INITIALIZED;

	g_palloc = allocator->pMalloc;
	g_pfree  = allocator->pFree;
	return USOCK_OK;
}

usock_err_t initCommon()
{
	if(!g_palloc)
	{
		g_palloc = malloc;
		g_pfree  = free;
	}
	g_initialized = 1;
	return USOCK_OK;
}

void initSockInfo(struct SockInfoNode *node, size_t bytes, const char *name)
{
	memset(node, 0, bytes);
	node->blockSize = bytes;
#ifdef _WIN32
	strncpy_s(node->name, MAX_SOCKET_NAME_LEN, name, MAX_SOCKET_NAME_LEN);
#else
	strncpy(node->name, name, MAX_SOCKET_NAME_LEN);
#endif

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
}

void freeNodeList()
{
	struct SockInfoNode *si = g_head;
	struct SockInfoNode *next;

	/* Free all allocated socket nodes */
	while(si)
	{
		next = si->next;
		usock_close_socket(si);
		usock_free_socket(si);
		si = next;
	}
}

#define GET_SOCK_INFO_FROM_HANDLE(SOCKINFO_T, HSOCK) (SOCKINFO_T*)(((unsigned char*)HSOCK) + sizeof(SockInfoNode))

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

typedef struct SockInfo
{
	SOCKET sockfd;
	struct addrinfo info;
	unsigned sockopt;
}SockInfo;

const size_t kSockNodeSize = sizeof(SockInfoNode) + sizeof(SockInfo);

int usock_initialize()
{
	WSADATA wsaData;
	int iResult;

	if(g_initialized)
		return USOCK_ERROR_ALREADY_INITIALIZED;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
		return USOCK_ERROR_INIT_FAILED;

	initCommon();

	return USOCK_OK;
}

void usock_release()
{
	freeNodeList();
	WSACleanup();
}

int translateDomain(usock_domain_t domain)
{
	const int winsockDomains[] = {
		AF_UNSPEC,
		AF_INET,
		AF_INET6
	};
	return winsockDomains[domain];
}

void translateProtocol(usock_socket_type_t type, int *outType, int *outProtocol)
{
	const int winsockProtocol[] = {
		IPPROTO_UDP,
		IPPROTO_TCP
	};

	const int winsockType[] = {
		SOCK_DGRAM,
		SOCK_STREAM
	};

	*outType = winsockType[type];
	*outProtocol = winsockProtocol[type];
}

void usock_configure(usock_handle_t hsock, usock_domain_t domain, usock_socket_type_t type, usock_flags_t flags)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	ZeroMemory(&node->info, sizeof(node->info));
	node->info.ai_family = translateDomain(domain);
	translateProtocol(type, &node->info.ai_socktype, &node->info.ai_protocol);
	node->sockopt = flags;
}

usock_err_t usock_bind(usock_handle_t hsock, usock_port_t port)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct addrinfo *result = NULL;
	#define PORT_STR_SIZE 8
	char portStr[PORT_STR_SIZE];
	int val, ret;

	// Resolve the server address and port
	node->info.ai_flags = AI_PASSIVE;
	val = sprintf_s(portStr, PORT_STR_SIZE, "%hu", port);
	portStr[val] = '\0';
	ret = getaddrinfo((PCSTR)NULL, portStr, &node->info, &result);
	if (ret != 0) {
		return USOCK_ERROR_INTERNAL;
	}

	node->sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (node->sockfd == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		return USOCK_ERROR_INIT_FAILED;
	}

	/* Set socket options */
	val = node->sockopt & USOCK_OPTIONS_REUSE_ADDRESS;
	setsockopt(node->sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&val, sizeof(val));
	/* SO_REUSEPORT doesn't seem to exist for windows */

	/* Bind the socket */
	ret = bind(node->sockfd, result->ai_addr, (int)result->ai_addrlen);
	freeaddrinfo(result);

	return ret == SOCKET_ERROR ? USOCK_ERROR_INIT_FAILED : USOCK_OK;
}

usock_err_t usock_listen(usock_handle_t hsock, int backlog)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	int ret;

	if(node->sockfd == INVALID_SOCKET)
		return USOCK_ERROR_NOT_INITIALIZED;

	ret = listen(node->sockfd, backlog);
	if(ret == SOCKET_ERROR)
		return USOCK_ERROR_INTERNAL;

	return USOCK_OK;
}

usock_err_t usock_accept(usock_handle_t hsock, usock_handle_t *pOutSock)
{
	SOCKET newSock;
	unsigned char address[sizeof(struct sockaddr_in6)];
	socklen_t len  = sizeof(address); /* We just use the biggest one for size. */

	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct SockInfo *outNode;

	newSock = accept(node->sockfd, (struct sockaddr *)address, &len);
	if(newSock == INVALID_SOCKET)
	{
		*pOutSock = NULL;
		return USOCK_ERROR_INTERNAL;
	}

	usock_create_socket("client socket", pOutSock);
	outNode = GET_SOCK_INFO_FROM_HANDLE(SockInfo, (*pOutSock));

	/* Cache the returned socket info */
	outNode->sockfd = newSock;
	memcpy(&outNode->info, address, len);

	return USOCK_OK;
}

usock_err_t usock_connect(usock_handle_t hsock, const char *ip_address, usock_port_t port)
{
	int ret, val;
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	char portStr[PORT_STR_SIZE];
	struct addrinfo *result;

	/* Resolve address info */
	val = sprintf_s(portStr, PORT_STR_SIZE, "%hu", port);
	portStr[val] = '\0';
	ret = getaddrinfo(ip_address, portStr, &node->info, &result);
	if (ret != 0) 
	{
		return USOCK_ERROR_INTERNAL;
	}

	/* open socket */
	node->sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(node->sockfd == INVALID_SOCKET)
	{
		return USOCK_ERROR_INIT_FAILED;
	}

	ret = connect(node->sockfd, result->ai_addr, (int)result->ai_addrlen);

	freeaddrinfo(result);

	if(ret == SOCKET_ERROR)
		return USOCK_ERROR_INTERNAL;
	return USOCK_OK;
}

usock_ssize_t usock_recv(usock_handle_t hsock, void *pOutBuffer, usock_size_t buflen)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	return (usock_ssize_t)recv(node->sockfd, (char*)pOutBuffer, (int)buflen, 0);
}

usock_ssize_t usock_send(usock_handle_t hsock, const void *buffer, usock_size_t buflen)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	return (usock_ssize_t)send(node->sockfd, (const char*)buffer, (int)buflen, 0);
}

usock_ssize_t usock_recv_from(usock_handle_t hsock, void * pBuffer, usock_size_t len, usock_flags_t flags, usock_handle_t * pOutClientInfo)
{
	socklen_t clilen = sizeof(struct sockaddr_in);
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct SockInfo *cliinfoNode;

	/* Allocate a node to hold the client info */
	if(pOutClientInfo)
	{
		usock_create_socket("", pOutClientInfo);
		cliinfoNode = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, (*pOutClientInfo));
		return (usock_ssize_t)recvfrom(node->sockfd, pBuffer, (int)len, (int)flags, (struct sockaddr *)&cliinfoNode->info, &clilen);
	}

	/* Receive the client message */
	return (usock_ssize_t)recvfrom(node->sockfd, pBuffer, (int)len, (int)flags, (struct sockaddr *)NULL, NULL);
}

usock_ssize_t usock_send_to(usock_handle_t hsock, const void *pBuffer, usock_size_t len, usock_flags_t flags, usock_handle_t hdest)
{
	struct SockInfo *srcNode = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct SockInfo *dstNode;
	struct sockaddr *info = NULL;
	int infolen = 0;
	if(hdest)
	{
		dstNode = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hdest);
		info = (struct sockaddr *)&dstNode->info;
		infolen = sizeof(struct sockaddr_in);
	}
	return (usock_ssize_t)sendto(srcNode->sockfd, pBuffer, (int)len, (unsigned)flags, info, infolen);
}

void usock_close_socket(usock_handle_t hsock)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	closesocket(node->sockfd);
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
#include <errno.h>

typedef struct SockInfo
{
	/* All required info about the socket. */
	int socketfd;
	struct sockaddr_in info;
	/* TODO: support other structs as well */
	int protocol;
	unsigned sockopt;
}SockInfo;

const size_t kSockNodeSize = sizeof(SockInfoNode) + sizeof(SockInfo);

usock_err_t usock_initialize()
{
	if(g_initialized)
		return USOCK_ERROR_ALREADY_INITIALIZED;

	return initCommon();
}

void usock_release()
{
	freeNodeList();
}

void usock_configure(usock_handle_t hsock, usock_domain_t domain, usock_socket_type_t type, uint32_t flags)
{
	struct SockInfo *info = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
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

usock_err_t usock_bind(usock_handle_t hsock, usock_port_t port)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	int ret;
	int val;

	node->socketfd = socket(node->info.sin_family, node->protocol, 0);
	if(node->socketfd < 0)
	{
		/* Error handling */ 
		return USOCK_ERROR_INIT_FAILED;
	}

	/* Set socket options */
	val = node->sockopt & USOCK_OPTIONS_REUSE_ADDRESS;
	ret = setsockopt(node->socketfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

	val = node->sockopt & USOCK_OPTIONS_REUSE_PORT;
	ret = setsockopt(node->socketfd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val));

	/* Bind the socket */
	node->info.sin_addr.s_addr = INADDR_ANY;
	node->info.sin_port = htons(port);

	ret = bind(node->socketfd, (struct sockaddr *)&node->info, sizeof(node->info));
	return ret < 0 ? USOCK_ERROR_INIT_FAILED : USOCK_OK;
}

usock_err_t usock_listen(usock_handle_t hsock, int backlog)
{
	int ret;
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	if(!node->socketfd)
	{
		return USOCK_ERROR_NOT_INITIALIZED;
	}


	ret = listen(node->socketfd, backlog);
	if(ret < 0)
	{
		return USOCK_ERROR_INTERNAL;
	}

	return USOCK_OK;
}

usock_err_t usock_accept(usock_handle_t hsock, usock_handle_t *pOutSock)
{
	int newSock;
	struct sockaddr_in address;
	socklen_t len = sizeof(address);
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct SockInfo *outNode;

	newSock = accept(node->socketfd, (struct sockaddr *)&address, &len);
	if(newSock < 0)
	{
		*pOutSock = NULL;
		return USOCK_ERROR_INTERNAL;
	}

	usock_create_socket("client socket", pOutSock);
	outNode = GET_SOCK_INFO_FROM_HANDLE(SockInfo, (*pOutSock));

	/* Cache the returned socket info */
	outNode->socketfd = newSock;
	memcpy(&outNode->info, &address, len);

	return USOCK_OK;
}

usock_err_t usock_connect( usock_handle_t hsock, const char *ip_address, unsigned short port )
{
	int ret;
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);

	/* open socket */
	node->socketfd = socket(node->info.sin_family, node->protocol, 0);
	if(node->socketfd < 0)
	{
		return USOCK_ERROR_INIT_FAILED;
	}

	node->info.sin_port = htons(port);
	if(inet_pton(node->info.sin_family, ip_address, &node->info.sin_addr) <= 0)
	{
		/* address parse failed */
		return USOCK_ERROR_INVALID_ARG;
	}

	ret = connect(node->socketfd, (struct sockaddr *)&node->info, sizeof(node->info));
	if(ret < 0)
	{
		/* connect failed */
		return USOCK_ERROR_INTERNAL;
	}

	return USOCK_OK;
}

usock_ssize_t usock_read(usock_handle_t hsock, void *pOutBuffer, usock_size_t buflen)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	return read(node->socketfd, pOutBuffer, buflen);
}

usock_ssize_t usock_send(usock_handle_t hsock, const void *buffer, usock_size_t buflen)
{
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	return send(node->socketfd, buffer, buflen, 0);
}

usock_ssize_t usock_recv_from(usock_handle_t hsock, void *pBuffer, usock_size_t len, unsigned flags, usock_handle_t *pOutClientInfo)
{
	socklen_t clilen = sizeof(struct sockaddr_in);
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct SockInfo *cliinfoNode;

	/* Allocate a node to hold the client info */
	if(pOutClientInfo)
	{
		usock_create_socket("", pOutClientInfo);
		cliinfoNode = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, (*pOutClientInfo));
		return recvfrom(node->socketfd, pBuffer, len, (int)flags, (struct sockaddr *)&cliinfoNode->info, &clilen);
	}

	/* Receive the client message */
	return recvfrom(node->socketfd, pBuffer, len, (int)flags, (struct sockaddr *)NULL, NULL);
}

usock_ssize_t usock_send_to(usock_handle_t hsock, const void *pBuffer, usock_size_t len, usock_flags_t flags, usock_handle_t hdest)
{
	struct SockInfo *srcNode = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	struct SockInfo *dstNode;
	struct sockaddr *info = NULL;
	size_t infolen = 0;
	if(hdest)
	{
		dstNode = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hdest);
		info = (struct sockaddr *)&dstNode->info;
		infolen = sizeof(struct sockaddr_in);
	}
	return sendto(srcNode->socketfd, pBuffer, len, (unsigned)flags, info, infolen);
}

void usock_close_socket(usock_handle_t hsock)
{
	/* Close the connection */
	struct SockInfo *node = GET_SOCK_INFO_FROM_HANDLE(struct SockInfo, hsock);
	if(node->socketfd)
		close(node->socketfd);
	node->socketfd = 0;
}

#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#   error "Unknown compiler"
#endif

/***************************************/
/*            COMMON CODE              */
/***************************************/

void printNodeList();

usock_err_t usock_create_socket(const char *name, usock_handle_t *pOutSocket)
{
	struct SockInfoNode *node = (struct SockInfoNode *)g_palloc(kSockNodeSize);
	if(!node)
	{
		*pOutSocket = NULL;
		return USOCK_ERROR_OUT_OF_MEMORY;
	}

	initSockInfo(node, kSockNodeSize, name);
	
	*pOutSocket = (void*)node;

	/* Debug code */
	printNodeList();

	return USOCK_OK;
}

usock_err_t usock_create_socket_ex(const char *name, usock_size_t userBytes, usock_handle_t *pOutSocket, void **ppOutUserData)
{
	struct SockInfoNode *node = (struct SockInfoNode *)g_palloc(kSockNodeSize + userBytes);
	if(!node)
	{
		*pOutSocket = NULL;
		*ppOutUserData = NULL;
		return USOCK_ERROR_OUT_OF_MEMORY;
	}

	initSockInfo(node, kSockNodeSize + userBytes, name);
	
	*pOutSocket    = (void*)node;
	*ppOutUserData = (void*)((unsigned char *)node + kSockNodeSize);

	/* Debug code */
	printNodeList();

	return USOCK_OK;
}

void usock_free_socket(usock_handle_t hsock)
{
	struct SockInfoNode *node = (struct SockInfoNode *)hsock;
	// struct SockInfo *node = (struct SockInfo *)hsock;

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
		/* 
		*  Freeing the tail node. 
		*  Update tail pointer.
		*/
		g_tail = g_tail->prev;
	}
	else if(node == g_head)
	{
		/* 
		*  Freeing the head node. 
		*  Update head pointer.
		*/
		g_head = g_head->next;
	}
	
	/* Free allocated node */
	g_pfree(node);

	/* Debug code */
	printNodeList();
}

void printNodeList()
{
	struct SockInfoNode *node = g_head;
	while(node)
	{
		/* printf("%s", node->name); */
		/* TODO: Provide callback for debug printing */
		if(node->next)
		{
			/* printf("->"); */
		}
		node = node->next;
	}
	/* printf("\n"); */
}
