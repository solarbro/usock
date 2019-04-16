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
* Optional bit flags for configuring the socket.
*/
typedef enum
{
	USOCK_OPTIONS_DEFAULT       = 0x0,
	USOCK_OPTIONS_REUSE_ADDRESS = 0x1,
	USOCK_OPTIONS_REUSE_PORT    = 0x2,
} usock_options;

/*
* The network socket handle.
* This is just an opaque pointer to an internal data structure.
*/
typedef void * usock_handle;

/*
* Initialize the usock library.
* This must be called before any other usock function.
* \return - Error code (see usock_err for more info)
*/
int usock_initialize();

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
* \param pOutSock - The returned socket handle.
* \return - Error code (see usock_err for more info)
*/
int usock_create_socket(
	const char       *name,
	usock_handle     *pOutSocket
);

/*
* Configure the connection protocol.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param domain - The connection domain (see usock_domain for more info).
* \param type - The connection type (see usock_socket_type for more info).
*/
void usock_configure(
	usock_handle      hsock, 
	usock_domain      domain, 
	usock_socket_type type,
	unsigned          flags
);

/*
* Bind the server socket to the specified port.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param port - The port number to bind the socket to.
* \return - Error code (see usock_err for more info)
*/
int usock_bind(
	usock_handle hsock, 
	unsigned     port
);

/*
* Listen for incoming connections.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param backlog - The maximum length that the queue of pending 
*                  connections for this socket can grow to.
* \return - Error code (see usock_err for more info)
*/
int usock_listen(
	usock_handle socket, 
	int          backlog
);

/*
* Extract the first connection request from the pending connections queue
* and create and return a socket for this connection.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param pOutSock - The returned socket handle.
* \return - Error code (see usock_err for more info)
*/
int usock_accept(
	usock_handle  hsock, 
	usock_handle *pOutSock
);

/*
* Connect to a server at the specified address and port.
* \param hsock - The socket handle (returned by usock_create_socket).
* \param ip_address - The ip address to connect to.
* \param port - The port to connect to.
* \return - Error code (see usock_err for more info)
*/
int usock_connect(
	usock_handle    hsock,
	const char     *ip_address,
	unsigned short  port
);

/*
* Read incoming data from the connected socket. This is a blocking call.
* \param hsock - The socket handle (returned by usock_socket).
* \param pOutBuffer - A buffer into which the incoming data will be put.
* \param buflen - The size of the provided buffer.
* \return - Error code (see usock_err for more info)
*/
int usock_read(
	usock_handle        hsock, 
	void               *pOutBuffer, 
	unsigned long long  buflen
);

/*
* Send a block of data to the connected socket.
* \param hsock - The socket handle (returned by usock_socket).
* \param pBuffer - The buffer containing the data to be sent.
* \param buflen - The number bytes to be sent.
* \return - Error code (see usock_err for more info)
*/
int usock_send(
	usock_handle        hsock, 
	const void         *pBuffer, 
	unsigned long long  buflen
);

/*
* Close the socket connection.
* \param hsock - The socket handle (returned by usock_socket).
*/
void usock_close_socket(
	usock_handle socket
);

/* 
* Release the memory associated with this socket.
* \param hsock - The socket handle (returned by usock_socket).
*/
void usock_free_socket(
	usock_handle hsock
);

/* TODO: Add debug callbacks */

#ifdef __cplusplus
}
#endif