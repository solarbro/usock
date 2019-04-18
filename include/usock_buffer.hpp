#pragma once
#include <cstdlib>

namespace usock
{
	/*
	* A buffer that usock C++ functions can fill out without 
	* worrying about allocated buffer size.
	*/
	class buffer
	{
	public:
		buffer();
		buffer(const void *data, size_t size);
		~buffer();
		buffer(const buffer &rhs);
		buffer(buffer &&rref);
		const buffer &operator=(const buffer &rhs);
		const buffer &operator=(buffer &&rref);

		buffer &append(const void *data, size_t bytes);
		template<typename T>
		buffer &append(const T &data)
		{
			//append
			return *this;
		}

	private:
		void *m_data;
		size_t m_size;
		size_t m_allocatedSize;
	};
}