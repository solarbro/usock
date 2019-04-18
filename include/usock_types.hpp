#pragma once
#include <usock.h>

namespace usock
{
	// Typedef enums to use C++ style syntax
	using err_t         = usock_err_t;
	using domain_t      = usock_domain_t;
	using socket_type_t = usock_socket_type_t;
	using options_t     = usock_options_t;

	// Additional typedefs
	using flags_t       = usock_flags_t;
	using port_t        = usock_port_t;
	using handle_t      = usock_handle_t;
}
