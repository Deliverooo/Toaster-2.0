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
#pragma once

#include "platform/os.hpp"

#include <Windows.h>

namespace tst
{
	class WindowsOS final : public OS
	{
	public:
		explicit WindowsOS(HINSTANCE hInstance);
		~WindowsOS() override;

		[[nodiscard]] const char *getName() const override;
		[[nodiscard]] const char *getVersionInfo() const override;

		[[nodiscard]] HINSTANCE getInstanceHandle() const;

		EError createProcess(const String &path, const String &args) override;
		bool   killProcess(int pid) override;
		bool   isProcessRunning(int pid) override;

		bool        hasEnvironmentVariable(const char *name) const override;
		const char *getEnvironmentVariable(const char *name) const override;
		void        setEnvironmentVariable(const char *name, const char *value) override;

		[[nodiscard]] const char *getTemporaryDirectory() const override;
		[[nodiscard]] const char *getUserDirectory() const override;

		void alertPopup(const char *title, const char *message) override;

		LPWSTR stringToLPWSTR(const String &input_str);

	private:
		HINSTANCE m_hInstance;
	};
}
