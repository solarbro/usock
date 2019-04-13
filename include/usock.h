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
	USOCK_DOMAIN_IPV4 = 0,
	USOCK_DOMAIN_IPV6,
} usock_domain;

/*
* The socket type to use.
* Fast - UDP connection, fast but unreliable.
* Reliable - TCP connection, more overhead than UDP but also more reliable.
*/
typedef enum
{
	USOCK_SOCKTYPE_FAST = 0,
	USOCK_SOCKTYPE_RELIABLE,
} usock_socket_type;

typedef struct usock_address_in
{
	short          family;
	unsigned short port;
	unsigned       addr;
	char           zero[8];
} usock_address_in;

typedef struct usock_address
{
	unsigned short family;
	char           data[14];
} usock_address;

typedef struct usock_addr_info
{
	int                     flags;
	usock_domain            family;
	usock_socket_type       protocol;
	unsigned long long      addrlen;
	struct usock_address   *addr;
	char                   *canonname;
	//struct usock_addr_info *next;
} usock_addr_info;

/*
* The network socket handle.
*/
typedef union usock_socket_handle
{
	void  *winsock;
	int    posixsock;
} usock_socket_handle;

typedef struct usock_socket
{
	usock_socket_handle hsock;
	unsigned reserved[2];
} usock_socket;

/*
* Initialize the usock library.
* This must be called before any other usock function.
* \return - error code (see usock_err for more info)
*/
int  usock_initialize();

/*
* Release the usock library.
* This should be the last usock function to be called.
* Call this before exiting the program.
*/
void usock_release();

/*
* Create a socket.
* \param outSocket - the returned socket handle.
* \param domain - the communication domain (IPV4 or IPV6)
* \param type - the communication type (fast - UDP, reliable - TCP)
* \return - error code (see usock_err for more info)
*/
int usock_open_socket(
	usock_socket     *pOutSocket, 
	usock_domain      domain, 
	usock_socket_type type
);

/*
*
*/
int usock_bind(
	const usock_socket  *socket, 
	unsigned             port
);

int usock_listen(
	const usock_socket *socket, 
	int                 backlog
);

int usock_accept(
	const usock_socket   *socket,
	struct usock_address *addr,
	unsigned             *addr_len
);

int usock_connect(
	const usock_socket *socket,
	const char         *ip_address,
	unsigned            port
);

int usock_close_socket(
	usock_socket *socket
);

#ifdef __cplusplus
}
#endif