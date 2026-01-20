#pragma once

#include <string>
#include "gl_enums.hpp"

namespace gl
{
	inline std::string to_string(Error p_value)
	{
		switch (p_value)
		{
			case Error::eNoError: return "No Error";
			case Error::eInvalidEnum: return "Invalid Enum";
			case Error::eInvalidValue: return "Invalid Value";
			case Error::eInvalidOperation: return "Invalid Operation";
			case Error::eStackOverflow: return "Stack Overflow";
			case Error::eStackUnderflow: return "Stack Underflow";
			case Error::eOutOfMemory: return "Out Of Memory";
			case Error::eInvalidFramebufferOperation: return "Invalid Framebuffer Operation";
			case Error::eContextLost: return "Context Lost";
			case Error::eTableTooLarge: return "Table Too Large";
		}
		return "";
	}

	inline std::string to_string(DebugSource p_value)
	{
		switch (p_value)
		{
			case DebugSource::eAPI: return "API";
			case DebugSource::eWindowSystem: return "Window System";
			case DebugSource::eShaderCompiler: return "Shader Compiler";
			case DebugSource::eThirdParty: return "Third Party";
			case DebugSource::eApplication: return "Application";
			case DebugSource::eOther: return "Other";
		}
		return "";
	}

	inline std::string to_string(DebugType p_value)
	{
		switch (p_value)
		{
			case DebugType::eError: return "Error";
			case DebugType::eDeprecatedBehavior: return "Deprecated Behavior";
			case DebugType::eUndefinedBehavior: return "Undefined Behavior";
			case DebugType::ePortability: return "Portability";
			case DebugType::ePerformance: return "Performance";
			case DebugType::eMarker: return "Marker";
			case DebugType::ePushGroup: return "Push Group";
			case DebugType::ePopGroup: return "Pop Group";
			case DebugType::eOther: return "Other";
		}
		return "";
	}

	inline std::string to_string(DebugSeverity p_value)
	{
		switch (p_value)
		{
			case DebugSeverity::eHigh: return "High";
			case DebugSeverity::eMedium: return "Medium";
			case DebugSeverity::eLow: return "Low";
			case DebugSeverity::eNotification: return "Notification";
		}
		return "";
	}
}
