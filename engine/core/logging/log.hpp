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

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#ifdef TST_PLATFORM_WINDOWS
#include <Windows.h>

#undef min
#undef max

#endif

namespace tst
{
	enum class ELogLevel
	{
		eTrace,
		eInfo,
		eWarn,
		eError,
		eFatal
	};

	class Log
	{
	public:
		struct TagDetails
		{
			bool      enabled     = true;
			ELogLevel levelFilter = ELogLevel::eTrace;
		};

		static void init();
		static void terminate();

		static void setDefaultTagSettings() { s_enabledTags = s_defaultTagDetails; }

		static std::shared_ptr<spdlog::logger> &getLogger() { return m_logger; }

		static bool                               hasTag(const std::string &tag) { return s_enabledTags.contains(tag); }
		static std::map<std::string, TagDetails> &enabledTags() { return s_enabledTags; }

		template<typename... Args>
		static void printMessage(ELogLevel level, fmt::format_string<Args...> format, Args &&... args);

		template<typename... Args>
		static void printMessageTag(ELogLevel level, std::string_view tag, fmt::format_string<Args...> format, Args &&... args);

		static void printMessageTag(ELogLevel level, std::string_view tag, std::string_view message);

		template<typename... Args>
		static void printAssertMessage(std::string_view prefix, fmt::format_string<Args...> message, Args &&... args);

		static void printAssertMessage(std::string_view prefix);

		static const char *levelToString(ELogLevel level)
		{
			switch (level)
			{
				case ELogLevel::eTrace: return "Trace";
				case ELogLevel::eInfo: return "Info";
				case ELogLevel::eWarn: return "Warn";
				case ELogLevel::eError: return "Error";
				case ELogLevel::eFatal: return "Fatal";
			}
			return "";
		}

		static ELogLevel levelFromString(std::string_view string)
		{
			if (string == "Trace")
			{
				return ELogLevel::eTrace;
			}
			if (string == "Info")
			{
				return ELogLevel::eInfo;
			}
			if (string == "Warn")
			{
				return ELogLevel::eWarn;
			}
			if (string == "Error")
			{
				return ELogLevel::eError;
			}
			if (string == "Fatal")
			{
				return ELogLevel::eFatal;
			}

			return ELogLevel::eTrace;
		}

	private:
		static std::shared_ptr<spdlog::logger> m_logger;

		inline static std::map<std::string, TagDetails> s_enabledTags;
		static std::map<std::string, TagDetails>        s_defaultTagDetails;
	};
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TST_TRACE_TAG(tag, ...) ::tst::Log::printMessageTag(::tst::ELogLevel::eTrace, tag, __VA_ARGS__)
#define TST_INFO_TAG(tag, ...) 	::tst::Log::printMessageTag(::tst::ELogLevel::eInfo, tag, __VA_ARGS__)
#define TST_WARN_TAG(tag, ...) 	::tst::Log::printMessageTag(::tst::ELogLevel::eWarn, tag, __VA_ARGS__)
#define TST_ERROR_TAG(tag, ...) ::tst::Log::printMessageTag(::tst::ELogLevel::eError, tag, __VA_ARGS__)
#define TST_FATAL_TAG(tag, ...) ::tst::Log::printMessageTag(::tst::ELogLevel::eFatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TST_TRACE(...) ::tst::Log::printMessage(::tst::ELogLevel::eTrace, __VA_ARGS__)
#define TST_INFO(...) 	::tst::Log::printMessage(::tst::ELogLevel::eInfo, __VA_ARGS__)
#define TST_WARN(...) 	::tst::Log::printMessage(::tst::ELogLevel::eWarn, __VA_ARGS__)
#define TST_ERROR(...) ::tst::Log::printMessage(::tst::ELogLevel::eError, __VA_ARGS__)
#define TST_FATAL(...) ::tst::Log::printMessage(::tst::ELogLevel::eFatal, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tst
{
	template<typename... Args>
	void Log::printMessage(ELogLevel level, fmt::format_string<Args...> format, Args &&... args)
	{
		auto detail = s_enabledTags[""];
		if (detail.enabled && detail.levelFilter <= level)
		{
			auto logger = getLogger();
			switch (level)
			{
				case ELogLevel::eTrace:
					logger->trace(format, std::forward<Args>(args)...);
					break;
				case ELogLevel::eInfo:
					logger->info(format, std::forward<Args>(args)...);
					break;
				case ELogLevel::eWarn:
					logger->warn(format, std::forward<Args>(args)...);
					break;
				case ELogLevel::eError:
					logger->error(format, std::forward<Args>(args)...);
					break;
				case ELogLevel::eFatal:
					logger->critical(format, std::forward<Args>(args)...);
					break;
			}
		}
	}

	template<typename... Args>
	void Log::printMessageTag(ELogLevel level, std::string_view tag, fmt::format_string<Args...> format, Args &&... args)
	{
		auto detail = s_enabledTags[std::string(tag)];
		if (detail.enabled && detail.levelFilter <= level)
		{
			auto logger = getLogger();

			std::string formatted = fmt::format(format, std::forward<Args>(args)...);
			switch (level)
			{
				case ELogLevel::eTrace:
					logger->trace("[{0}] {1}", tag, formatted);
					break;
				case ELogLevel::eInfo:
					logger->info("[{0}] {1}", tag, formatted);
					break;
				case ELogLevel::eWarn:
					logger->warn("[{0}] {1}", tag, formatted);
					break;
				case ELogLevel::eError:
					logger->error("[{0}] {1}", tag, formatted);
					break;
				case ELogLevel::eFatal:
					logger->critical("[{0}] {1}", tag, formatted);
					break;
			}
		}
	}

	inline void Log::printMessageTag(ELogLevel level, std::string_view tag, std::string_view message)
	{
		auto detail = s_enabledTags[std::string(tag)];
		if (detail.enabled && detail.levelFilter <= level)
		{
			auto logger = getLogger();
			switch (level)
			{
				case ELogLevel::eTrace:
					logger->trace("[{0}] {1}", tag, message);
					break;
				case ELogLevel::eInfo:
					logger->info("[{0}] {1}", tag, message);
					break;
				case ELogLevel::eWarn:
					logger->warn("[{0}] {1}", tag, message);
					break;
				case ELogLevel::eError:
					logger->error("[{0}] {1}", tag, message);
					break;
				case ELogLevel::eFatal:
					logger->critical("[{0}] {1}", tag, message);
					break;
			}
		}
	}

	template<typename... Args>
	void Log::printAssertMessage(std::string_view prefix, fmt::format_string<Args...> message, Args &&... args)
	{
		auto logger = getLogger();

		auto formatted = fmt::format(message, std::forward<Args>(args)...);
		logger->error("{0}: {1}", prefix, formatted);
		#ifdef TST_PLATFORM_WINDOWS
		MessageBoxA(nullptr, formatted.data(), "Toaster Assert", MB_OK | MB_ICONERROR);
		#endif
	}

	inline void Log::printAssertMessage(std::string_view prefix)
	{
		auto logger = getLogger();

		logger->error("{0}", prefix);
		#ifdef TST_PLATFORM_WINDOWS
		MessageBoxA(nullptr, "No message :(", "Toaster Assert", MB_OK | MB_ICONERROR);
		#endif
	}
}
