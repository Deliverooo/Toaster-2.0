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

#include "core/error.hpp"
#include "core/string.hpp"

namespace tst
{
	class OS
	{
	public:
		OS();
		virtual ~OS() = default;

		static OS *get();

		[[nodiscard]] virtual const char *getName() const = 0;
		[[nodiscard]] virtual const char *getVersionInfo() const = 0;

		virtual EError createProcess(const String &path, const String &args) = 0;
		virtual bool   killProcess(int pid) = 0;
		virtual bool   isProcessRunning(int pid) = 0;

		virtual bool        hasEnvironmentVariable(const char *name) const = 0;
		virtual const char *getEnvironmentVariable(const char *name) const = 0;
		virtual void        setEnvironmentVariable(const char *name, const char *value) = 0;

		[[nodiscard]] virtual const char *getTemporaryDirectory() const = 0;
		[[nodiscard]] virtual const char *getUserDirectory() const = 0;

		virtual void alertPopup(const char *title, const char *message) = 0;
	};
}
