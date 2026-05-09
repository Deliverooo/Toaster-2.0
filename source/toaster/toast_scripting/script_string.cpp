#include "script_string.hpp"

namespace toaster::script
{
	String::String(ScriptEngine *p_engine) : m_engine(p_engine)
	{
		m_string = mono_string_empty(m_engine->getAppDomain());
	}

	String::String(ScriptEngine *p_engine, const toaster::String &p_text) : m_engine(p_engine)
	{
		m_string = mono_string_new(m_engine->getAppDomain(), p_text.c_str());
	}

	String::~String()
	{
		if (m_data)
			mono_free(m_data);
	}

	MonoString *String::operator&()
	{
		return m_string;
	}

	auto String::getMonoString() const -> MonoString *
	{
		return m_string;
	}

	auto String::getString() -> toaster::String
	{
		if (!m_data)
			m_data = mono_string_to_utf8(m_string);
		return m_data;
	}

	auto String::getData() const -> char *
	{
		return m_data;
	}
}
