#pragma once

#include "script_engine.hpp"
#include "toast_lib/ptr.hpp"
#include "toast_lib/type_traits.hpp"

namespace toaster::script
{
	class TST_API Class
	{
	public:
		Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name);
		Class(ScriptEngine *p_engine, MonoClass *p_class);

		template<typename... TArgs>
		auto invokeStaticMethod(const String &p_method_name, const TArgs &... p_args) -> MonoObject *
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0u)
			{
				void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};

				MonoMethod *method{mono_class_get_method_from_name(m_class, p_method_name.c_str(), parameter_count)};
				return mono_runtime_invoke(method, nullptr, params, nullptr);
			}
			else
			{
				MonoMethod *method{mono_class_get_method_from_name(m_class, p_method_name.c_str(), parameter_count)};
				MonoObject *ret{mono_runtime_invoke(method, nullptr, nullptr, nullptr)};
				return ret;
			}
		}

		template<typename... TArgs>
		auto invokeStaticMethod(MonoMethod *p_method, const TArgs &... p_args) -> MonoObject *
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0u)
			{
				void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};
				return mono_runtime_invoke(p_method, nullptr, params, nullptr);
			}
			else
			{
				MonoObject *ret{mono_runtime_invoke(p_method, nullptr, nullptr, nullptr)};
				return ret;
			}
		}

		[[nodiscard]] auto getMethod(const String &p_method_name, int32 p_param_count) -> MonoMethod *;
		[[nodiscard]] auto getClass() -> MonoClass *;
		[[nodiscard]] auto getScriptEngine() -> ScriptEngine *;

	private:
		ScriptEngine *m_engine{nullptr};

		MonoClass *m_class{nullptr};

		friend class Object;
	};

	class TST_API Object
	{
	public:
		Object(Class *p_class);
		Object(MonoObject *p_object);

		// You must call this before using any methods, it is not in the constructor so you can deffer the initialisation.
		template<typename... TArgs>
		void construct(TArgs &&... p_args)
		{
			invoke(".ctor", std::forward<TArgs>(p_args)...);
		}

		template<typename... TArgs>
		auto invoke(const String &p_method_name, TArgs &&... p_args) -> MonoObject *
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0u)
			{
				void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};

				MonoMethod *method{mono_class_get_method_from_name(m_class->m_class, p_method_name.c_str(), parameter_count)};
				return mono_runtime_invoke(method, m_object, params, nullptr);
			}
			else
			{
				MonoMethod *method{mono_class_get_method_from_name(m_class->m_class, p_method_name.c_str(), parameter_count)};
				return mono_runtime_invoke(method, m_object, nullptr, nullptr);
			}
		}

		template<typename... TArgs>
		auto invoke(MonoMethod *p_method, TArgs &&... p_args) -> MonoObject *
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0u)
			{
				void *params[parameter_count]{( getAddressIfNotPointer(p_args), ...)};
				return mono_runtime_invoke(p_method, m_object, params, nullptr);
			}
			else
			{
				return mono_runtime_invoke(p_method, m_object, nullptr, nullptr);
			}
		}

		template<typename Type>
		auto castTo() -> Type *
		{
			return (Type *) mono_object_unbox(m_object);
		}

		[[nodiscard]] auto getClass() -> Class *;
		[[nodiscard]] auto getObject() -> MonoObject *;

	private:
		NonOwningPtr<Class> m_class{nullptr};

		MonoObject *m_object{nullptr};
	};
}
