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
#include <atomic>
#include <new>

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
		explicit unique_sock(handle_t hsock)
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

		handle_t release()
		{
			handle_t tmp = m_handle;
			m_handle = nullptr;
			return tmp;
		}

		void reset(handle_t hsock)
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
		shared_sock()
		{
			// Allocate socket node with extra memory for the reference counter.
			usock_create_socket_ex("", sizeof(refcount_t), &m_handle, (void**)&m_refCounter);
			// Call the constructor to initialize to 1.
			new (m_refCounter) refcount_t(1);
		}

		shared_sock(const char *name)
		{
			// Allocate socket node with extra memory for the reference counter.
			usock_create_socket_ex(name, sizeof(refcount_t), &m_handle, (void**)&m_refCounter);
			// Call the constructor to initialize to 1.
			new (m_refCounter) refcount_t(1);
		}

		~shared_sock()
		{
			release();
		}

		shared_sock(const shared_sock &rhs)
		{
			release();
			m_handle = rhs.m_handle;
			m_refCounter = rhs.m_refCounter;
			++(*m_refCounter);
		}

		const shared_sock &operator=(const shared_sock &rhs)
		{
			release();
			m_handle = rhs.m_handle;
			m_refCounter = rhs.m_refCounter;
			++(*m_refCounter);
			return *this;
		}

		void swap(shared_sock &rhs)
		{
			std::swap(m_handle    , rhs.m_handle    );
			std::swap(m_refCounter, rhs.m_refCounter);
		}

		int64_t use_count() const
		{
			if(!m_refCounter)
				return 0;
			return (*m_refCounter);
		}

		bool is_unique() const
		{
			return use_count() == 1;
		}

	private:
		using refcount_t = std::atomic_int64_t;
		refcount_t *m_refCounter;

		void release()
		{
			if(m_refCounter)
			{
				--(*m_refCounter);
				if(*m_refCounter <= 0)
				{
					usock_close_socket(m_handle);
					usock_free_socket(m_handle);
					m_handle = nullptr;
				}
				m_refCounter = nullptr;
			}
		}
	};
}