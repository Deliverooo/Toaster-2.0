#pragma once

#include "script_engine.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::script
{
	class TST_API String
	{
	public:
		String(ScriptEngine *p_engine);
		String(ScriptEngine *p_engine, const toaster::String &p_text);
		~String();

		MonoString *operator&();

		auto getData() const -> char *;
		auto getString() -> toaster::String;
		auto getMonoString() const -> MonoString *;

	private:
		NonOwningPtr<ScriptEngine> m_engine{nullptr};

		char *      m_data{nullptr};
		MonoString *m_string{nullptr};
	};
}
