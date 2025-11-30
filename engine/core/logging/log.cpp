#include "log.hpp"

#include <filesystem>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace tst
{
	std::shared_ptr<spdlog::logger> Log::m_logger;

	std::map<std::string, Log::TagDetails> Log::s_defaultTagDetails = {
		{"Platform", TagDetails{true, ELogLevel::eWarn}}, {"Animation", TagDetails{true, ELogLevel::eWarn}}, {"Audio", TagDetails{true, ELogLevel::eInfo}},
		{"Core", TagDetails{true, ELogLevel::eTrace}}, {"Memory", TagDetails{true, ELogLevel::eError}}, {"Renderer", TagDetails{true, ELogLevel::eInfo}},
		{"Scene", TagDetails{true, ELogLevel::eInfo}},
	};

	void Log::init()
	{
		std::string logsDirectory = "logs";
		if (!std::filesystem::exists(logsDirectory))
		{
			std::filesystem::create_directories(logsDirectory);
		}

		std::vector<spdlog::sink_ptr> toasterSinks = {
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Toaster.tlog", true), std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
		};

		toasterSinks[0]->set_pattern("[%T] [%l] %n: %v");
		toasterSinks[1]->set_pattern("%^[%T] %n: %v%$");

		m_logger = std::make_shared<spdlog::logger>("Toaster", toasterSinks.begin(), toasterSinks.end());
		m_logger->set_level(spdlog::level::trace);

		setDefaultTagSettings();
	}

	void Log::terminate()
	{
		m_logger.reset();
		spdlog::drop_all();
	}
}
