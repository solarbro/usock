//C++ object-oriented version of the usock library

#include <usock.h>

namespace usock
{
	/*
	* A simple RAII wrapper for the usock library.
	* Just create an instance to initialize the library.
	* The library will be automatically shut down when the app exits.
	*/
	class Instance
	{
	public:
		Instance()
		{
			usock_initialize();
		}

		~Instance()
		{
			usock_release();
		}
	};
}