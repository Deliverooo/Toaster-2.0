#include "script_object.hpp"

#include "toast_lib/toast_assert.h"

namespace toaster::script
{
	Class::Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name, EClassScope p_class_scope) : m_engine(p_engine), m_classScope(p_class_scope)
	{
		m_class = mono_class_from_name((m_classScope == EClassScope::eApp) ? m_engine->getAppImage() : m_engine->getCoreImage(), p_namespace.c_str(), p_name.c_str());
	}

	Class::Class(ScriptEngine *p_engine, MonoClass *p_class, EClassScope p_class_scope) : m_engine(p_engine), m_class(p_class), m_classScope(p_class_scope)
	{
	}

	auto Class::getMethod(const String &p_method_name, uint32 p_parameter_count) -> MonoMethod *
	{
		return mono_class_get_method_from_name(m_class, p_method_name.c_str(), p_parameter_count);
	}

	auto Class::getClass() -> MonoClass *
	{
		return m_class;
	}

	auto Class::getScriptEngine() -> ScriptEngine *
	{
		return m_engine;
	}

	auto Class::getClassScope() const -> EClassScope
	{
		return m_classScope;
	}

	Object::Object(const Class &p_class) : m_class(p_class)
	{
		m_object = mono_object_new(m_class.m_engine->getAppDomain(), m_class.m_class);
	}

	Object::Object(ScriptEngine *p_engine, MonoObject *p_object) : m_object(p_object)
	{
		MonoClass *klass{mono_object_get_class(p_object)};
		m_class = Class{p_engine, klass};
	}

	auto Object::getMethod(const String &p_method_name, uint32 p_parameter_count) const -> Method *
	{
		return mono_class_get_method_from_name(m_class.m_class, p_method_name.c_str(), p_parameter_count);
	}

	auto Object::getClass() -> Class &
	{
		return m_class;
	}

	auto Object::getObject() -> MonoObject *
	{
		return m_object;
	}
}
