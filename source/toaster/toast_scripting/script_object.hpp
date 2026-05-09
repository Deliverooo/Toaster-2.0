#pragma once

#include "script_common.hpp"
#include "script_engine.hpp"
#include "script_string.hpp"
#include "toast_lib/ptr.hpp"
#include "toast_lib/type_traits.hpp"

namespace toaster::script
{
	class Class;
	class Object;

	class TST_API Class
	{
	public:
		Class() = default;
		Class(ScriptEngine *p_engine, const toaster::String &p_namespace, const toaster::String &p_name, EClassScope p_class_scope = EClassScope::eApp);
		Class(ScriptEngine *p_engine, MonoClass *p_class, EClassScope p_class_scope = EClassScope::eApp);

		template<typename... TArgs>
		auto invokeStaticMethod(const toaster::String &p_method_name, const TArgs &... p_args) -> std::optional<Object>; // Optional because some function return void
		template<typename... TArgs>
		auto invokeStaticMethod(Method *p_method, const TArgs &... p_args) -> std::optional<Object>; // Optional because some function return void

		template<typename Type>
		auto getStaticFieldValue(const toaster::String &p_field_name, Type &p_out_value) -> void;
		template<typename Type>
		auto setStaticFieldValue(const toaster::String &p_field_name, const Type &p_value) -> void;

		template<typename Type>
		auto getStaticFieldValue(MonoClassField *p_field, Type &p_out_value) -> void;
		template<typename Type>
		auto setStaticFieldValue(MonoClassField *p_field, const Type &p_value) -> void;

		[[nodiscard]] auto getMethod(const toaster::String &p_method_name, uint32 p_parameter_count) const -> Method *;
		[[nodiscard]] auto getClass() const -> MonoClass *;
		[[nodiscard]] auto getVTable() const -> MonoVTable *;
		[[nodiscard]] auto getScriptEngine() -> NonOwningPtr<ScriptEngine>;
		[[nodiscard]] auto getClassScope() const -> EClassScope;
		[[nodiscard]] auto getNamespace() const -> const toaster::String &;
		[[nodiscard]] auto getName() const -> const toaster::String &;

	private:
		NonOwningPtr<ScriptEngine> m_engine{nullptr};
		MonoClass *                m_class{nullptr};
		MonoVTable *               m_vTable{nullptr};

		toaster::String m_namespace;
		toaster::String m_name;
		EClassScope     m_classScope{EClassScope::eApp};

		friend class Object;
	};

	class TST_API Object
	{
	public:
		Object() = default;
		Object(const Class &p_class);
		Object(ScriptEngine *p_engine, MonoObject *p_object); // Allow for construction via a mono object, useful for return values of "invoke"

		// You must call this before using any methods, it is not in the constructor so you can deffer the initialisation.
		template<typename... TArgs>
		void construct(TArgs &&... p_args)
		{
			invoke(".ctor", std::forward<TArgs>(p_args)...);
		}

		template<typename... TArgs>
		auto invoke(const toaster::String &p_method_name, TArgs &&... p_args) -> std::optional<Object> // Optional because some function return void
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0u)
			{
				void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};

				Method *    method{getMethod(p_method_name, parameter_count)};
				MonoObject *result{mono_runtime_invoke(method, m_object, params, nullptr)};
				if (result)
					return Object{m_class.m_engine, result};
				return std::nullopt;
			}
			else
			{
				Method *    method{getMethod(p_method_name, parameter_count)};
				MonoObject *result{mono_runtime_invoke(method, m_object, nullptr, nullptr)};
				if (result)
					return Object{m_class.m_engine, result};
				return std::nullopt;
			}
		}

		template<typename... TArgs>
		auto invoke(Method *p_method, TArgs &&... p_args) -> std::optional<Object> // Optional because some function return void
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0u)
			{
				void *      params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};
				MonoObject *result{mono_runtime_invoke(p_method, m_object, params, nullptr)};
				if (result)
					return Object{m_class.m_engine, result};
				return std::nullopt;
			}
			else
			{
				MonoObject *result{mono_runtime_invoke(p_method, m_object, nullptr, nullptr)};
				if (result)
					return Object{m_class.m_engine, result};
				return std::nullopt;
			}
		}

		template<typename Type>
		auto castTo() -> Type *
		{
			return (Type *) mono_object_unbox(m_object);
		}

		[[nodiscard]] auto getMethod(const toaster::String &p_method_name, uint32 p_parameter_count) const -> Method *;
		[[nodiscard]] auto getClass() -> Class &;
		[[nodiscard]] auto getObject() const -> MonoObject *;

	private:
		Class m_class;

		MonoObject *m_object{nullptr};
	};

	template<typename Type>
	auto Class::getStaticFieldValue(const toaster::String &p_field_name, Type &p_out_value) -> void
	{
		MonoClassField *field{mono_class_get_field_from_name(m_class, p_field_name.c_str())};

		if constexpr (std::is_same_v<Type, String>)
		{
			mono_field_static_get_value(m_vTable, field, ((String &) p_out_value).getMonoString());
		}
		else
		{
			mono_field_static_get_value(m_vTable, field, &p_out_value);
		}
	}

	template<typename Type>
	auto Class::setStaticFieldValue(const toaster::String &p_field_name, const Type &p_value) -> void
	{
		MonoClassField *field{mono_class_get_field_from_name(m_class, p_field_name.c_str())};

		if constexpr (std::is_same_v<Type, String>)
		{
			mono_field_static_set_value(m_vTable, field, ((const String &) p_value).getMonoString());
		}
		else
		{
			mono_field_static_set_value(m_vTable, field, getAddressIfNotPointer(p_value));
		}
	}

	template<typename Type>
	auto Class::getStaticFieldValue(MonoClassField *p_field, Type &p_out_value) -> void
	{
		mono_field_static_get_value(m_vTable, p_field, &p_out_value);
	}

	template<typename Type>
	auto Class::setStaticFieldValue(MonoClassField *p_field, const Type &p_value) -> void
	{
		mono_field_static_set_value(m_vTable, p_field, getAddressIfNotPointer(p_value));
	}

	template<typename... TArgs>
	auto Class::invokeStaticMethod(const toaster::String &p_method_name, const TArgs &... p_args) -> std::optional<Object>
	{
		constexpr uint32 parameter_count{sizeof...(p_args)};
		if constexpr (parameter_count > 0u)
		{
			void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};

			Method *    method{mono_class_get_method_from_name(m_class, p_method_name.c_str(), parameter_count)};
			MonoObject *result{mono_runtime_invoke(method, nullptr, params, nullptr)};
			if (result)
				return Object{m_engine, result};
			return std::nullopt;
		}
		else
		{
			Method *    method{mono_class_get_method_from_name(m_class, p_method_name.c_str(), parameter_count)};
			MonoObject *result{mono_runtime_invoke(method, nullptr, nullptr, nullptr)};
			if (result)
				return Object{m_engine, result};
			return std::nullopt;
		}
	}

	template<typename... TArgs>
	auto Class::invokeStaticMethod(Method *p_method, const TArgs &... p_args) -> std::optional<Object>
	{
		constexpr uint32 parameter_count{sizeof...(p_args)};
		if constexpr (parameter_count > 0u)
		{
			void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};

			MonoObject *result{mono_runtime_invoke(p_method, nullptr, params, nullptr)};
			if (result)
				return Object{m_engine, result};
			return std::nullopt;
		}
		else
		{
			MonoObject *result{mono_runtime_invoke(p_method, nullptr, nullptr, nullptr)};
			if (result)
				return Object{m_engine, result};
			return std::nullopt;
		}
	}
}
