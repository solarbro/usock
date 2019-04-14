#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
	
/*
* USock error codes. Functions that require error handling will return
* one of these error codes.
*/
typedef enum 
{
	USOCK_OK = 0,
	USOCK_ERROR_INIT_FAILED,
	USOCK_ERROR_MIN_REQUIREMENTS_MISSING,
	USOCK_ERROR_NOT_INITIALIZED,
	USOCK_ERROR_OUT_OF_MEMORY,
	USOCK_ERROR_NETWORK_DOWN,
	USOCK_ERROR_INVALID_ARG,
	USOCK_ERROR_PROTOCOL_NOT_SUPPORTED,
	USOCK_ERROR_INTERNAL, //Use usock_get_last_error() to get internal error code
} usock_err;

/*
* The communication domain to use.
*/
typedef enum
{
	USOCK_UNSPECIFIED = 0,
	USOCK_DOMAIN_IPV4,
	USOCK_DOMAIN_IPV6,
} usock_domain;

/*
* The socket type to use.
* Fast     - UDP connection, fast but unreliable.
* Reliable - TCP connection, more overhead than UDP but also more reliable.
*/
typedef enum
{
	USOCK_SOCKTYPE_FAST = 0,
	USOCK_SOCKTYPE_RELIABLE,
} usock_socket_type;

/*
* The network socket handle.
*/
typedef void * usock_handle;

/*
* Initialize the usock library.
* This must be called before any other usock function.
* \return - error code (see usock_err for more info)
*/
int usock_initialize();

/*
* Release the usock library.
* This should be the last usock function to be called.
* Call this before exiting the program.
*/
void usock_release();

/*
* Create a socket.
* \param outSocket - the returned socket handle.
* \return - error code (see usock_err for more info)
*/
int usock_create_socket(
	const char       *name,
	usock_handle     *pOutSocket
);

/*
*	Configure the connection protocol.
*/
void usock_configure(
	usock_handle hsock, 
	usock_domain domain, 
	usock_socket_type type
);

/*
* Bind the server socket to the specified port.
*/
int usock_bind(
	usock_handle hsocket, 
	unsigned     port
);

/*
* Listen for incoming connections.
*/
int usock_listen(
	usock_handle socket, 
	int          backlog
);

/*
*
*/
int usock_accept(
	usock_handle hsock, 
	usock_handle *pOutSock
);

/*
* Connect to a server at the specified address and port.
*/
int usock_connect(
	usock_handle    socket,
	const char     *ip_address,
	unsigned short  port
);

/*
*
*/
int usock_read(
	usock_handle        hsock, 
	void               *pOutBuffer, 
	unsigned long long  buflen
);

/*
*
*/
int usock_send(
	usock_handle        hsock, 
	const void         *buffer, 
	unsigned long long  buflen
);

/*
* Close the socket connection
*/
void usock_close_socket(
	usock_handle socket
);

/* 
* Release the memory allocated for this socket 
*/
void usock_free_socket(
	usock_handle hsock
);

/* TODO: Add debug callbacks */

#ifdef __cplusplus
}
#endif