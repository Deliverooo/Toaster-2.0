#include <fmt/format.h>
#include <fmt/color.h>

namespace tst
{
	enum class ELogLevel
	{
		eInfo,
		eTrace,
		eWarning,
		eError,
		eCritical
	};

	template<typename... Args>
	void _printMessage(ELogLevel level, fmt::format_string<Args...> format_str, Args &&... args)
	{
		const std::string message = fmt::format(format_str, std::forward<Args>(args)...);

		switch (level)
		{
			case ELogLevel::eInfo:
				fmt::print(fmt::fg(fmt::rgb(0, 0, 255)), "[INFO]: {}\n", message);
				break;
			case ELogLevel::eTrace:
				fmt::print(fmt::fg(fmt::rgb(0, 255, 255)), "[TRACE]: {}\n", message);
				break;
			case ELogLevel::eWarning:
				fmt::print(fmt::fg(fmt::rgb(255, 255, 0)), "[WARNING]: {}\n", message);
				break;
			case ELogLevel::eError:
				fmt::print(fmt::fg(fmt::rgb(255, 0, 0)), "[ERROR]: {}\n", message);
				break;
			case ELogLevel::eCritical:
				fmt::print(fmt::fg(fmt::rgb(255, 0, 255)), "[CRITICAL]: {}\n", message);
				break;
			default:
				fmt::print(fmt::fg(fmt::rgb(255, 255, 255)), "[UNKNOWN]: {}\n", message);
				break;
		}
	}

	#define TST_LOG_INFO(format_str, ...) ::tst::_printMessage(::tst::ELogLevel::eInfo, format_str, ##__VA_ARGS__)
	#define TST_LOG_TRACE(format_str, ...) ::tst::_printMessage(::tst::ELogLevel::eTrace, format_str, ##__VA_ARGS__)
	#define TST_LOG_WARNING(format_str, ...) ::tst::_printMessage(::tst::ELogLevel::eWarning, format_str, ##__VA_ARGS__)
	#define TST_LOG_ERROR(format_str, ...) ::tst::_printMessage(::tst::ELogLevel::eError, format_str, ##__VA_ARGS__)
	#define TST_LOG_CRITICAL(format_str, ...) ::tst::_printMessage(::tst::ELogLevel::eCritical, format_str, ##__VA_ARGS__)
}
