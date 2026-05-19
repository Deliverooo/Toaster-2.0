#pragma once

#include <fmt/color.h>
#include <fmt/format.h>
#include <glm/glm.hpp>

template<>
struct fmt::formatter<glm::vec2>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const glm::vec2 &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {})", v.x, v.y);
	}
};

template<>
struct fmt::formatter<glm::vec3>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const glm::vec3 &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
	}
};

template<>
struct fmt::formatter<glm::vec4>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const glm::vec4 &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
	}
};

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

	template<ELogLevel log_level, typename... TArgs>
	auto printMessage(fmt::format_string<TArgs...> format, TArgs &&... args) -> void
	{
		std::string formatted = fmt::format(format, std::forward<TArgs>(args)...);
		if constexpr (log_level == ELogLevel::eTrace)
		{
			fmt::print(fmt::fg(fmt::terminal_color::cyan), "{}\n", formatted);
		}
		else if constexpr (log_level == ELogLevel::eInfo)
		{
			fmt::print(fmt::fg(fmt::terminal_color::bright_green), "{}\n", formatted);
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

	#ifndef NDEBUG

	// Debug - disabled in release builds
	#define DEBUG_LOG_TRACE(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eTrace>(__VA_ARGS__)
	#define DEBUG_LOG_INFO(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eInfo>(__VA_ARGS__);
	#define DEBUG_LOG_WARN(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eWarning>(__VA_ARGS__)
	#define DEBUG_LOG_ERROR(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eError>(__VA_ARGS__)
	#define DEBUG_LOG_FATAL(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eFatal>(__VA_ARGS__)
	#else
	#define LOGD_TRACE(...)
	#define LOGD_INFO(...)
	#define LOGD_WARN(...)
	#define LOGD_ERROR(...)
	#define LOGD_FATAL(...)
	#endif

	#define LOG_TRACE(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eTrace>(__VA_ARGS__)
	#define LOG_INFO(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eInfo>(__VA_ARGS__)
	#define LOG_WARN(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eWarning>(__VA_ARGS__)
	#define LOG_ERROR(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eError>(__VA_ARGS__)
	#define LOG_FATAL(...) ::toaster::log::printMessage<::toaster::log::ELogLevel::eFatal>(__VA_ARGS__)
}
