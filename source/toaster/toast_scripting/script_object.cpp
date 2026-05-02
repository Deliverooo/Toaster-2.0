#include "script_object.hpp"

namespace toaster::script
{
	Class::Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name) : m_engine(p_engine)
	{
		m_class = mono_class_from_name(m_engine->getImage(), p_namespace.c_str(), p_name.c_str());
	}

	Object::Object(Class *p_class) : m_class(p_class)
	{
		m_object = mono_object_new(m_class->m_engine->getAppDomain(), m_class->m_class);
	}
}
