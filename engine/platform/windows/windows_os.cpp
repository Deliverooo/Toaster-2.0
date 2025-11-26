#include "platform/windows/windows_os.hpp"

#include <iostream>
#include <ostream>
#include <sstream>

namespace tst
{
	WindowsOS::~WindowsOS() = default;

	WindowsOS::WindowsOS(HINSTANCE hInstance)
	{
		m_hInstance = hInstance;
	}

	const char *WindowsOS::getName() const
	{
		return "Windows";
	}

	const char *WindowsOS::getVersionInfo() const
	{
		typedef void (WINAPI*RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

		HMODULE hNtdll = ::LoadLibrary("ntdll.dll");

		if (hNtdll == nullptr)
		{
			std::wcerr << L"Error loading ntdll.dll" << std::endl;
			return "";
		}

		RtlGetVersionPtr rtlGetVersionFunc = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hNtdll, "RtlGetVersion"));
		if (rtlGetVersionFunc == nullptr)
		{
			std::wcerr << L"Error getting RtlGetVersion function address" << std::endl;
			::FreeLibrary(hNtdll);
			return "";
		}

		RTL_OSVERSIONINFOW osVersion  = {0};
		osVersion.dwOSVersionInfoSize = sizeof(osVersion);

		rtlGetVersionFunc(&osVersion);

		std::ostringstream ss;
		ss << "OS Version Information:" << std::endl;
		ss << "Major Version: " << osVersion.dwMajorVersion << std::endl;
		ss << "Minor Version: " << osVersion.dwMinorVersion << std::endl;
		ss << "Build Number: " << osVersion.dwBuildNumber << std::endl;

		::FreeLibrary(hNtdll);

		return ss.str().c_str();
	}

	HINSTANCE WindowsOS::getInstanceHandle() const
	{
		return m_hInstance;
	}

	EError WindowsOS::createProcess(const std::string &path, const std::string &args)
	{
		STARTUPINFO         si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		// You can set additional startup properties here, e.g., si.dwFlags, si.wShowWindow
		ZeroMemory(&pi, sizeof(pi));

		// Create the child process
		if (!CreateProcess(nullptr,                          // No module name (use command line)
						   const_cast<char *>(path.c_str()), // Command line
						   nullptr,                          // Process handle not inheritable
						   nullptr,                          // Thread handle not inheritable
						   false,                            // Set handle inheritance to FALSE
						   0,                                // Creation flags (e.g., CREATE_NEW_CONSOLE to give it its own console)
						   nullptr,                          // Use parent's environment block
						   nullptr,                          // Use parent's starting directory
						   &si,                              // Pointer to STARTUPINFO structure
						   &pi)                              // Pointer to PROCESS_INFORMATION structure
		)
		{
			return EError::eUnknown;
		}

		// Wait until child process exits (optional, remove this line if the parent should not wait)
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return EError::eOk;
	}

	bool WindowsOS::killProcess(int pid)
	{
		return false;
	}

	bool WindowsOS::isProcessRunning(int pid)
	{
		return false;
	}

	bool WindowsOS::hasEnvironmentVariable(const char *name) const
	{
		return getenv(name) != nullptr;
	}

	const char *WindowsOS::getEnvironmentVariable(const char *name) const
	{
		return getenv(name);
	}

	void WindowsOS::setEnvironmentVariable(const char *name, const char *value)
	{
		::SetEnvironmentVariable(name, value);
	}

	const char *WindowsOS::getTemporaryDirectory() const
	{
		return getenv("TEMP");
	}

	const char *WindowsOS::getUserDirectory() const
	{
		return getenv("USERPROFILE");
	}

	void WindowsOS::alertPopup(const char *title, const char *message)
	{
		::MessageBoxA(nullptr, message, title, MB_OK | MB_ICONINFORMATION);
	}

	LPWSTR WindowsOS::stringToLPWSTR(const std::string &input_str)
	{
		// 1. Determine the required buffer size
		int buffer_size = MultiByteToWideChar(CP_UTF8,           // Use UTF-8 code page for input std::string
											  0,                 // No special flags
											  input_str.c_str(), // Input string
											  -1,                // Input string is null-terminated
											  NULL,              // Output buffer (NULL for size calculation)
											  0                  // Output buffer size (0 for size calculation)
											 );

		if (buffer_size == 0)
		{
			// Handle error (e.g., GetLastError())
			return nullptr;
		}

		// 2. Allocate the wide character buffer (use a vector for automatic memory management)
		// Note: LPWSTR requires a writable buffer, so we use new[] and return the pointer
		// The caller is responsible for deleting this memory!
		wchar_t *wide_buffer = new wchar_t[buffer_size];

		// 3. Perform the actual conversion
		MultiByteToWideChar(CP_UTF8, 0, input_str.c_str(), -1, wide_buffer, buffer_size);

		return wide_buffer;
	}
}
