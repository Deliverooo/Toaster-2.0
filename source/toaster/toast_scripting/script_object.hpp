#pragma once

#include "script_engine.hpp"
#include "toast_lib/type_traits.hpp"

namespace toaster::script
{
	class TST_API Class
	{
	public:
		Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name);
		Class(ScriptEngine *p_engine, MonoClass *p_class);

		template<typename... TArgs>
		MonoObject *invokeStaticMethod(const String &p_method_name, const TArgs &... p_args)
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

		[[nodiscard]] auto getClass() -> MonoClass *;

	private:
		ScriptEngine *m_engine{nullptr};

		MonoClass *m_class{nullptr};

		friend class Object;
	};

	class TST_API Object
	{
	public:
		Object(Class *p_class);

		template<typename... TArgs>
		void construct(TArgs &&... p_args)
		{
			invoke(".ctor", std::forward<TArgs>(p_args)...);
		}

		template<typename... TArgs>
		MonoObject *invoke(const String &p_method_name, TArgs &&... p_args)
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

		[[nodiscard]] auto getClass() -> Class *;
		[[nodiscard]] auto getObject() -> MonoObject *;

	private:
		Class *m_class{nullptr};

		MonoObject *m_object{nullptr};
	};
}
