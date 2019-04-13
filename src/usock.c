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
#elif __unix__ // all unices not caught above
// Unix
#elif defined(_POSIX_VERSION)
// POSIX
#else
#   error "Unknown compiler"
#endif


