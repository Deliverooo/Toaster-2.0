#include "os.hpp"

#include <print>

namespace tst
{
	static OS *singleton = nullptr;

	OS::OS()
	{
		singleton = this;
	}

	OS *OS::get()
	{
		return singleton;
	}
}
