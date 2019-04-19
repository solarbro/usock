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
#ifndef USOCK_H
#define USOCK_H

#include <stddef.h>

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
	USOCK_ERROR_ALREADY_INITIALIZED,
	USOCK_ERROR_OUT_OF_MEMORY,
	USOCK_ERROR_NETWORK_DOWN,
	USOCK_ERROR_INVALID_ARG,
	USOCK_ERROR_PROTOCOL_NOT_SUPPORTED,
	USOCK_ERROR_INTERNAL, //Use usock_get_last_error() to get internal error code
} usock_err_t;

/*
* The communication domain to use.
*/
typedef enum
{
	USOCK_DOMAIN_UNSPECIFIED = 0,
	USOCK_DOMAIN_IPV4,
	USOCK_DOMAIN_IPV6,
} usock_domain_t;

/*
* The socket type to use.
* Fast     - UDP connection, fast but unreliable.
* Reliable - TCP connection, more overhead than UDP but also more reliable.
*/
typedef enum
{
	USOCK_SOCKTYPE_FAST = 0,
	USOCK_SOCKTYPE_RELIABLE,
} usock_socket_type_t;

/*
* Optional bit flags for configuring the socket.
*/
typedef enum
{
	USOCK_OPTIONS_DEFAULT       = 0x0,
	USOCK_OPTIONS_REUSE_ADDRESS = 0x1,
	USOCK_OPTIONS_REUSE_PORT    = 0x2,
} usock_options_t;

/*
* Data type for passing bit flags.
*/
typedef unsigned int usock_flags_t;

/*
* Data type for passing port numbers.
*/
typedef unsigned short usock_port_t;

/*
* Data type for passing size and offset values.
* This is an UNSIGNED type.
*/
typedef unsigned long long usock_size_t;

/*
* Data type for passing size and offset values.
* This is an SIGNED type.
*/
typedef long long          usock_ssize_t;

/*
* The network socket handle.
* This is just an opaque pointer to an internal data structure.
*/
typedef void * usock_handle_t;

/*
* Custom allocator callbacks.
*/
typedef void *(*usock_palloc_t)(size_t bytes);
typedef void  (*usock_pfree_t)(void *ptr);

/*
* A struct to pass the custom allocate and free function pointers.
*/
typedef struct
{
	usock_palloc_t pMalloc;
	usock_pfree_t  pFree;
} usock_allocator;

/*
* Use this function to override the default allocator.
* This can only be used before usock_initialize() as overriding
* the allocator mid-run would cause memory leaks.
*/
usock_err_t usock_set_custom_allocator(const usock_allocator *pAllocator);

/*
* Initialize the usock library.
* This must be called before any other usock function.
* \return - Error code (see usock_err for more info)
*/
usock_err_t usock_initialize();

/*
* Release the usock library.
* This should be the last usock function to be called.
* Call this before exiting the program.
* Note: This call will automatically cleanup and free all sockets
* currently in use, so there's no need to explicitly cleanup any
* of the sockets while exiting.
*/
void usock_release();

/*
* Create a socket.
* \param name - An optional string name for this socket.
*               This can be useful for debugging.
* \param pOutSocket - The returned socket handle.
* \return - Error code (see usock_err_t for more info)
*/
usock_err_t usock_create_socket(
	const char         *name,
	usock_handle_t     *pOutSocket
);

/*
* Create a socket, and allocate some extra memory for the user.
* The extra bytes are for general use by the program.
* \param name - An optional string name for this socket.
*               This can be useful for debugging.
* \param userBytes - Number of extra bytes to allocate.
* \param pOutSocket - The returned socket handle.
* \param ppOutUserData - A pointer to the allocated user data.
* \return - Error code (see usock_err_t for more info).
*/
usock_err_t usock_create_socket_ex(
	const char        *name,
	usock_size_t       userBytes,
	usock_handle_t    *pOutSocket,
	void             **ppOutUserData
);

/*
* Configure the connection protocol.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param domain - The connection domain (see usock_domain for more info).
* \param type - The connection type (see usock_socket_type for more info).
*/
void usock_configure(
	usock_handle_t      hsock, 
	usock_domain_t      domain, 
	usock_socket_type_t type,
	usock_flags_t       flags
);

/*
* Bind the server socket to the specified port.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param port - The port number to bind the socket to.
* \return - Error code (see usock_err_t for more info)
*/
usock_err_t usock_bind(
	usock_handle_t      hsock, 
	usock_port_t        port
);

/*
* Listen for incoming connections.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param backlog - The maximum length that the queue of pending 
*                  connections for this socket can grow to.
* \return - Error code (see usock_err_t for more info)
*/
usock_err_t usock_listen(
	usock_handle_t      hsock, 
	int                 backlog
);

/*
* Extract the first connection request from the pending connections queue
* and create and return a socket for this connection.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param pOutSock - The returned socket handle.
* \return - Error code (see usock_err_t for more info)
*/
usock_err_t usock_accept(
	usock_handle_t      hsock, 
	usock_handle_t     *pOutSock
);

/*
* Connect to a server at the specified address and port.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param ip_address - The ip address to connect to.
* \param port - The port to connect to.
* \return - Error code (see usock_err_t for more info)
*/
usock_err_t usock_connect(
	usock_handle_t      hsock,
	const char         *ip_address,
	usock_port_t        port
);

/*
* Read incoming data from the connected socket. This is a blocking call.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param pOutBuffer - A buffer into which the incoming data will be put.
* \param buflen - The size of the provided buffer.
* \return - Number of bytes read.
*/
usock_ssize_t usock_recv(
	usock_handle_t      hsock, 
	void               *pOutBuffer, 
	usock_size_t        buflen
);

/*
* Send a block of data to the connected socket.
* \param hsock   - The socket handle (returned by usock_create_socket).
* \param pBuffer - The buffer containing the data to be sent.
* \param buflen  - The number bytes to be sent.
* \return        - Number of bytes sent.
*/
usock_ssize_t usock_send(
	usock_handle_t      hsock, 
	const void         *pBuffer, 
	usock_size_t        buflen
);

/*
* Receive a message.
* \param hsock          - The socket handle (returned by usock_create_socket)
* \param pBuffer        - The buffer to capture the incoming message in.
* \param len            - The size of the buffer.
* \param flags          - Options to configure the behavior of this function.
* \param pOutClientInfo - Handle to the sender of this message.
* \return               - Number of bytes received.
*/
usock_ssize_t usock_recv_from(
	usock_handle_t     hsock, 
	void              *pBuffer,
	usock_size_t       len,
	usock_flags_t      flags,
	usock_handle_t    *pOutClientInfo
);

/*
* Send a message to the specified recipient.
* \param hsock   - The socket handle (returned by usock_create_info)
* \param pBuffer - The buffer containing the message to be sent.
* \param len     - The size of the message buffer.
* \param flags   - Options to configure the behavior of this function.
* \param hdest   - Handle to the intended recipient of this message 
*                  (returned by usock_recv_from).
* \return        - Number of bytes sent.
*/
usock_ssize_t usock_send_to(
	usock_handle_t     hsock,
	const void        *pBuffer,
	usock_size_t       len,
	usock_flags_t      flags,
	usock_handle_t     hdest
);

/*
* Close the socket connection.
* \param hsock - The socket handle (returned by usock_create_socket).
*/
void usock_close_socket(
	usock_handle_t     socket
);

/* 
* Release the memory associated with this socket.
* \param hsock - The socket handle (returned by usock_create_socket).
*/
void usock_free_socket(
	usock_handle_t     hsock
);

/* TODO: Add debug callbacks */

#ifdef __cplusplus
}
#endif

#endif /* USOCK_H */