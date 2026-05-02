#pragma once
#include "script_engine.hpp"

namespace toaster::script
{
	class Class
	{
	public:
		Class(ScriptEngine *p_engine, const String &p_namespace, const String &p_name);

		template<typename... TArgs>
		MonoObject *invokeStaticMethod(const String &p_method_name, const TArgs &... p_args)
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0)
			{
				void *params[parameter_count]{(void *) &p_args...};

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

	private:
		ScriptEngine *m_engine{nullptr};

		MonoClass *m_class{nullptr};

		friend class Object;
	};

	class Object
	{
	public:
		Object(Class *p_class);

		template<typename... TArgs>
		void construct(const TArgs &... p_args)
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0)
			{
				void *params[parameter_count]{(void *) &p_args...};

				MonoMethod *method{mono_class_get_method_from_name(m_class->m_class, ".ctor", parameter_count)};
				mono_runtime_invoke(method, m_object, params, nullptr);
			}
			else
			{
				MonoMethod *method{mono_class_get_method_from_name(m_class->m_class, ".ctor", parameter_count)};
				mono_runtime_invoke(method, m_object, nullptr, nullptr);
			}
		}

		template<typename... TArgs>
		MonoObject *invoke(const String &p_method_name, const TArgs &... p_args)
		{
			constexpr uint32 parameter_count{sizeof...(p_args)};
			if constexpr (parameter_count > 0)
			{
				void *params[parameter_count]{(void *) &p_args...};

				MonoMethod *method{mono_class_get_method_from_name(m_class->m_class, p_method_name.c_str(), parameter_count)};
				return mono_runtime_invoke(method, m_object, params, nullptr);
			}
			else
			{
				MonoMethod *method{mono_class_get_method_from_name(m_class->m_class, p_method_name.c_str(), parameter_count)};
				return mono_runtime_invoke(method, m_object, nullptr, nullptr);
			}
		}

	private:
		Class *m_class{nullptr};

		MonoObject *m_object{nullptr};
	};
}
