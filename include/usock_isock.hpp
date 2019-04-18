#pragma once
#include <usock.h>
#include <usock_types.hpp>
#include <usock_buffer.hpp>

namespace usock
{
	/*
	* The socket interface wrapped in a class.
	* This isn't meant to be used directly. It's just a 
	* helper class for other RAII wrapper classes.
	*/
	class isock
	{
	public:
		void configure(
			domain_t domain,
			socket_type_t type,
			flags_t flags
		);

		err_t bind(
			port_t port
		);

		err_t listen(
			int backlog
		);

		err_t connect(
			const char *ip_address,
			port_t port
		);

		err_t read(
			buffer &outBuffer
		);

		err_t send(
			const buffer &buffer
		);

		handle_t recv_from(
			buffer &outBuffer,
			flags_t flags,
			err_t outError
		);

		err_t send_to(
			const buffer &buffer,
			flags_t flags,
			handle_t hdest
		);

	protected:
		handle_t m_handle;
	};
}