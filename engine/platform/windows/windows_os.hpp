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

		EError createProcess(const std::string &path, const std::string &args) override;
		bool   killProcess(int pid) override;
		bool   isProcessRunning(int pid) override;

		bool        hasEnvironmentVariable(const char *name) const override;
		const char *getEnvironmentVariable(const char *name) const override;
		void        setEnvironmentVariable(const char *name, const char *value) override;

		[[nodiscard]] const char *getTemporaryDirectory() const override;
		[[nodiscard]] const char *getUserDirectory() const override;

		void alertPopup(const char *title, const char *message) override;

		LPWSTR stringToLPWSTR(const std::string &input_str);

	private:
		HINSTANCE m_hInstance;
	};
}
