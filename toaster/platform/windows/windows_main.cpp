/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#include <Windows.h>

#include "allocator.hpp"
#include "log.hpp"
#include "platform/windows/windows_os.hpp"
#include "platform/application.hpp"

namespace tst
{
	extern Application *createApplication(const std::vector<String> &args);

	int _main(int argc, char **argv)
	{
		WindowsOS os{GetModuleHandle(nullptr)};

		std::vector<String> args;
		for (int i = 0; i < argc; i++)
		{
			args.emplace_back(argv[i]);
		}

		Log::init();

		Application *main_app = createApplication(args);
		TST_ASSERT(main_app);

		main_app->run();

		tdelete main_app;

		Log::terminate();

		return 0;
	}
}

#ifndef TST_DIST

int main(int argc, char **argv)
{
	return tst::_main(__argc, __argv);
}

#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	return tst::_main(__argc, __argv);
}
#endif
