#pragma once

#include <fmt/color.h>
#include <fmt/format.h>

namespace toaster::log
{
	enum class ELogLevel
	{
		eTrace,
		eInfo,
		eWarning,
		eError,
		eFatal
	};

	template<ELogLevel log_level, typename... Args>
	void printMessage(fmt::format_string<Args...> format, Args &&... args)
	{
		std::string formatted = fmt::format(format, std::forward<Args>(args)...);
		if constexpr (log_level == ELogLevel::eTrace)
		{
			fmt::print(fmt::fg(fmt::terminal_color::cyan), "{}\n", formatted);
		}
		else if constexpr (log_level == ELogLevel::eInfo)
		{
			fmt::print(fmt::fg(fmt::terminal_color::green), "{}\n", formatted);
		}
		else if constexpr (log_level == ELogLevel::eWarning)
		{
			fmt::print(fmt::fg(fmt::terminal_color::yellow), "{}\n", formatted);
		}
		else if constexpr (log_level == ELogLevel::eError)
		{
			fmt::print(fmt::fg(fmt::terminal_color::red), "{}\n", formatted);
		}
		else if constexpr (log_level == ELogLevel::eFatal)
		{
			fmt::print(fmt::fg(fmt::terminal_color::bright_red), "{}\n", formatted);
		}
	}

	#define LOG_TRACE(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eTrace>(__VA_ARGS__)
	#define LOG_INFO(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eInfo>(__VA_ARGS__)
	#define LOG_WARN(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eWarning>(__VA_ARGS__)
	#define LOG_ERROR(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eError>(__VA_ARGS__)
	#define LOG_FATAL(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eFatal>(__VA_ARGS__)
}
