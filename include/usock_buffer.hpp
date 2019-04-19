#pragma once
#include <cstdlib>

namespace usock
{
	template<typename T>
	class proxy_buffer
	{
	public:
		proxy_buffer() : m_start(nullptr), m_end(nullptr) {}
		proxy_buffer(const T *start, const T *end) : m_start(data), m_end(end) {}

		const T *data() const
		{
			return m_start;
		}

		// The proxy buffer has a type, so it's possible to have iterators!
		class iterator
		{
		public:
			iterator() : m_ptr(nullptr) {}
			iterator(const T *ptr) : m_ptr(ptr) {}

			// prefix
			const iterator& operator++()
			{
				++m_ptr;
				return *this;
			}
			// postfix
			iterator operator++(int)
			{
				iterator tmp(*this);
				operator++(); // prefix-increment this instance
				return tmp;   // return value before increment
			}

			iterator operator+(int offset)
			{
				return iterator(m_ptr + offset);
			}

			iterator &operator+=(int offset)
			{
				m_ptr += offset;
				return *this;
			}

			bool operator==(const iterator &iter)
			{
				return iter.m_ptr == m_ptr;
			}

			bool operator!=(const iterator &iter)
			{
				return iter.m_ptr != m_ptr;
			}

		private:
			const T *m_ptr;
		};

		iterator begin() const
		{
			return iterator(m_start);
		}

		iterator end() const
		{
			return iterator(m_end);
		}

	private:
		const T *m_start, *m_end;

	};

	/*
	* A buffer that usock C++ functions can fill out without 
	* worrying about allocated buffer size.
	*/
	class buffer
	{
	public:
		/*
		* Default constructor. Nothing is allocated.
		*/
		buffer();
		/*
		* Construct from a raw data buffer.
		* This will allocate and copy the data.
		*/
		buffer(const void *data, size_t size);
		/*
		* Copy constructor. 
		* This will create a copy of the data.
		*/
		buffer(const buffer &rhs);
		/*
		* Move constructor.
		* This will replace the data with the contents of rref.
		* Note: rref will be empty after this.
		*/
		buffer(buffer &&rref);
		/*
		* Copy assignment.
		* This will copy the contents of rhs.
		*/
		const buffer &operator=(const buffer &rhs);
		/*
		* Move assignment.
		* This will replace the data with the contents of rref.
		* Note: rref will be empty after this.
		*/
		const buffer &operator=(buffer &&rref);
		/*
		* Destructor. 
		* This will clean up the allocated memory.
		*/
		~buffer();

		/*
		* Append the contents of the raw data buffer.
		* The append functions return a reference to the class instance
		* so it's possible to chain them:
		* myBuffer.append(someData).append(someMoreData);
		*/
		buffer &append(const void *data, size_t bytes);
		buffer &append(const buffer &rhs);
		/*
		* Append a typed element to this buffer.
		* This is only there for convenience; the container won't 
		* keep track of the data type after this call.
		*/
		template<typename T>
		buffer &append(const T &data)
		{
			return append((const void*)&data, sizeof(T));
		}

		/*
		* Reserve the specified amount of memory. This won't have 
		* any effect if more there's already enough memory allocated.
		*/
		void reserve(size_t size);

		/*
		* Access the data.
		*/
		const void * data() const;
		      void * data();

		/*
		* Convenience function for type casting the data.
		* Usage: const MyType *p = mybuffer.cast<MyType>.data();
		* It can also be used to create an iterator.
		* Usage: auto iter = mybuffer.cast<MyType>.begin();
		*/
		template <typename T>
		proxy_buffer<T> cast() const
		{
			return proxy_buffer<T>(reinterpret_cast<const T*>(m_data));
		}

		/*
		* Check the current size of the buffer (not the total allocated size).
		*/
		size_t size() const;

	private:
		void *m_data;
		size_t m_size;
		size_t m_allocatedSize;
	};
}