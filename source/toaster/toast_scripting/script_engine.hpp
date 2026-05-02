#pragma once

#include "../toaster_macros.hpp"

#include "toast_lib/io/filesystem.hpp"

#include <mono/jit/jit.h>

namespace toaster::script
{
	struct TST_API ScriptEngineSpecInfo
	{
		String rootDomainName{"Root"};
		String appDomainName{"App"};

		io::filesystem::Path assemblyPath{};
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

		[[nodiscard]] auto getRootDomain() const -> MonoDomain *;
		[[nodiscard]] auto getAppDomain() const -> MonoDomain *;
		[[nodiscard]] auto getAssembly() const -> MonoAssembly *;
		[[nodiscard]] auto getImage() const -> MonoImage *;

		[[nodiscard]] auto loadAssembly(const io::filesystem::Path &p_path) const -> MonoAssembly *;

		auto printAssemblyTypes(MonoAssembly *p_assembly) const -> void;

	private:
		ScriptEngineSpecInfo m_specInfo{};

		MonoDomain *m_rootDomain{nullptr};
		MonoDomain *m_appDomain{nullptr};

		MonoAssembly *m_assembly{nullptr};
		MonoImage *   m_image{nullptr};
	};
}
