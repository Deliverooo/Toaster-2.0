#include "script_object.hpp"

namespace toaster::script
{
	Class::Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name) : m_engine(p_engine)
	{
		m_class = mono_class_from_name(m_engine->getImage(), p_namespace.c_str(), p_name.c_str());
	}

	Class::Class(ScriptEngine *p_engine, MonoClass *p_class) : m_engine(p_engine), m_class(p_class)
	{
	}

	auto Class::getMethod(const String &p_method_name, int32 p_param_count) -> MonoMethod *
	{
		return mono_class_get_method_from_name(m_class, p_method_name.c_str(), p_param_count);
	}

	auto Class::getClass() -> MonoClass *
	{
		return m_class;
	}

	auto Class::getScriptEngine() -> ScriptEngine *
	{
		return m_engine;
	}

	Object::Object(Class *p_class) : m_class(p_class)
	{
		m_object = mono_object_new(m_class->m_engine->getAppDomain(), m_class->m_class);
	}

	Object::Object(MonoObject *p_object) : m_object(p_object)
	{
	}

	auto Object::getClass() -> Class *
	{
		return m_class;
	}

	auto Object::getObject() -> MonoObject *
	{
		return m_object;
	}
}
