#pragma once

#include "core/error.hpp"

#include <string>

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

		virtual EError createProcess(const std::string &path, const std::string &args) = 0;
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
