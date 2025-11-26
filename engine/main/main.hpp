#pragma once

#include "core/error.hpp"

namespace tst
{
	class Main
	{
	public:
		static EError init();
		static EError start();
		static bool   loop();
		static void   cleanup();
	};
}
