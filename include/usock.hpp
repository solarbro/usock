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
#include <usock_types.hpp>
#include <usock_isock.hpp>
#include <usock_buffer.hpp>

namespace usock
{

	/*
	* A simple RAII wrapper for the usock library.
	* Just create an instance to initialize the library.
	* The library will be automatically shut down when the app exits.
	*/
	class instance
	{
	public:
		instance()
		{
			usock_initialize();
		}

		~instance()
		{
			usock_release();
		}
	};

	/*
	* RAII wrapper for the socket handle.
	* The handle is automatically allocated when declared.
	* Move semantics are used to ensure the handle is unique.
	*/
	class unique_sock : public isock
	{
	public:
		unique_sock()
		{
			// Allocate socket when created
			// Use empty name since one wan't provided
			usock_create_socket("", &m_handle);
		}

		unique_sock(const char *name)
		{
			// Allocate socket when created, using the provided name.
			usock_create_socket(name, &m_handle);
		}

		// Constructor that accepts a pre-allocated handle.
		explicit unique_sock(usock_handle_t hsock)
		{
			reset(hsock);
		}

		~unique_sock()
		{
			// Close and free socket when out of scope.
			reset(nullptr);
		}

		// Provide move semantics
		unique_sock(unique_sock &&handle)
		{
			reset(handle.release());
		}
		void operator=(unique_sock &&handle)
		{
			reset(handle.release());
		}

		// No copy constructor or assignment operator
		unique_sock(const unique_sock &handle) = delete;
		void operator=(const unique_sock &handle) = delete;

		usock_handle_t release()
		{
			usock_handle_t tmp = m_handle;
			m_handle = nullptr;
			return tmp;
		}

		void reset(usock_handle_t hsock)
		{
			if(m_handle)
			{
				usock_close_socket(m_handle);
				usock_free_socket(m_handle);
			}

			m_handle = hsock;
		}

	};

	/*
	* Another RAII wrapper; this one's reference counted.
	* The socket will only be freed when there are no more references left.
	* Use this with caution, as the reference counting comes with an overhead.
	*/
	class shared_sock : public isock
	{
	public:
		shared_sock();
		~shared_sock();

		shared_sock(const shared_sock &rhs);
		const shared_sock &operator=(const shared_sock &rhs);

		explicit shared_sock(handle_t hsock);

	};
}