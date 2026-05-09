#pragma once

#include "../toaster_macros.hpp"

#include "toast_lib/io/filesystem.hpp"

#include <mono/jit/jit.h>

#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <nethost.h>

#include "toast_lib/ptr.hpp"

namespace toaster::script
{
	struct TST_API ScriptEngineSpecInfo
	{
		String rootDomainName{"Root"};
		String appDomainName{"App"};

		io::filesystem::Path coreAssemblyPath{};
		io::filesystem::Path appAssemblyPath{};
	};

	// Specifies whether the class is a part of the core assembly or the app assembly
	enum class EClassScope
	{
		eCore, eApp
	};

	class TST_API ScriptEngine
	{
	public:
		ScriptEngine(const ScriptEngineSpecInfo &p_spec_info);
		~ScriptEngine();

		template<typename TFunc>
		auto registerMethod(const String &p_method_name, TFunc p_method) -> void
		{
			mono_add_internal_call(p_method_name.c_str(), (const void *) p_method);
		}

		[[nodiscard]] auto getSpecInfo() const -> const ScriptEngineSpecInfo &;
		[[nodiscard]] auto getRootDomain() const -> MonoDomain *;
		[[nodiscard]] auto getAppDomain() const -> MonoDomain *;

		[[nodiscard]] auto getCoreAssembly() const -> MonoAssembly *;
		[[nodiscard]] auto getCoreImage() const -> MonoImage *;
		[[nodiscard]] auto getAppAssembly() const -> MonoAssembly *;
		[[nodiscard]] auto getAppImage() const -> MonoImage *;

		[[nodiscard]] auto loadAssembly(const io::filesystem::Path &p_path) const -> MonoAssembly *;

		auto printAssemblyTypes(MonoAssembly *p_assembly) const -> void;

	private:
		ScriptEngineSpecInfo m_specInfo{};

		MonoDomain *m_rootDomain{nullptr};
		MonoDomain *m_appDomain{nullptr};

		MonoAssembly *m_coreAssembly{nullptr};
		MonoAssembly *m_appAssembly{nullptr};

		MonoImage *m_coreImage{nullptr};
		MonoImage *m_appImage{nullptr};
	};

	namespace clr
	{
		struct TST_API CLRScriptEngineSpecInfo
		{
			io::filesystem::Path coreAssemblyPath{};
			io::filesystem::Path appAssemblyPath{};
		};

		class TST_API CLRScriptEngine
		{
		public:
			CLRScriptEngine(const CLRScriptEngineSpecInfo &p_spec_info);
			~CLRScriptEngine();

			template<typename TFunc>
			auto getFunctionPointer(const toaster::String &p_type_name, const toaster::String &p_method_name) -> TFunc
			{
				TFunc func{nullptr};
				m_getFunctionPointerFn(convertUtf8ToWide(p_type_name).c_str(), convertUtf8ToWide(p_method_name).c_str(), UNMANAGEDCALLERSONLY_METHOD, nullptr, nullptr,
									   (void **) &func);

				return func;
			}

		private:
			CLRScriptEngineSpecInfo m_specInfo{};

			hostfxr_handle m_hostFxrContext{nullptr};

			hostfxr_initialize_for_runtime_config_fn m_initFn{nullptr};
			hostfxr_get_runtime_delegate_fn          m_getRuntimeDelegateFn{nullptr};
			hostfxr_close_fn                         m_closeFn{nullptr};

			load_assembly_fn        m_loadAssemblyFn{nullptr};
			get_function_pointer_fn m_getFunctionPointerFn{nullptr};
		};


	}
}
