#include "toast_script/script_object.hpp"

namespace toaster::script
{
	Class::Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name, const EClassScope p_class_scope) : m_engine(p_engine),
																																			   m_namespace(p_namespace),
																																			   m_name(p_name),
																																			   m_classScope(p_class_scope)
	{
		m_class  = mono_class_from_name((m_classScope == EClassScope::eApp) ? m_engine->getAppImage() : m_engine->getCoreImage(), p_namespace.c_str(), p_name.c_str());
		m_vTable = mono_class_vtable(m_engine->getAppDomain(), m_class);
	}

	Class::Class(ScriptEngine *p_engine, MonoClass *p_class, const EClassScope p_class_scope) : m_engine(p_engine), m_class(p_class),
																								m_namespace(mono_class_get_namespace(p_class)),
																								m_name(mono_class_get_name(p_class)), m_classScope(p_class_scope)
	{
		m_vTable = mono_class_vtable(m_engine->getAppDomain(), m_class);
	}

	auto Class::getMethod(const String &p_method_name, const uint32 p_parameter_count) const -> MonoMethod *
	{
		return mono_class_get_method_from_name(m_class, p_method_name.c_str(), p_parameter_count);
	}

	auto Class::getField(const String &p_field_name) const -> MonoClassField *
	{
		return mono_class_get_field_from_name(m_class, p_field_name.c_str());
	}

	auto Class::getClass() const -> MonoClass *
	{
		return m_class;
	}

	auto Class::getVTable() const -> MonoVTable *
	{
		return m_vTable;
	}

	auto Class::getScriptEngine() -> NonOwningPtr<ScriptEngine>
	{
		return m_engine;
	}

	auto Class::getClassScope() const -> EClassScope
	{
		return m_classScope;
	}

	auto Class::getNamespace() const -> const String &
	{
		return m_namespace;
	}

	auto Class::getName() const -> const String &
	{
		return m_name;
	}

	Object::Object(const Class &p_class) : m_class(p_class)
	{
		m_object = mono_object_new(m_class.m_engine->getAppDomain(), m_class.m_class);
	}

	Object::Object(ScriptEngine *p_engine, MonoObject *p_object) : m_object(p_object)
	{
		MonoClass *klass{mono_object_get_class(p_object)};
		m_class = Class{p_engine, klass};

		mono_class_get_name(klass);
	}

	auto Object::getMethod(const String &p_method_name, uint32 p_parameter_count) const -> Method *
	{
		return mono_class_get_method_from_name(m_class.m_class, p_method_name.c_str(), p_parameter_count);
	}

	auto Object::getVirtualMethod(Method *p_base_method) const -> Method *
	{
		return mono_object_get_virtual_method(m_object, p_base_method);
	}

	auto Object::overridesMethod(Method *p_base_method) const -> bool
	{
		Method *resolve_method{getVirtualMethod(p_base_method)};
		return p_base_method != resolve_method;
	}

	auto Object::getClass() -> Class &
	{
		return m_class;
	}

	auto Object::getObject() const -> MonoObject *
	{
		return m_object;
	}
}
