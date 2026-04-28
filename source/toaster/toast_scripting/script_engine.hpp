#pragma once

#include "../toaster_macros.hpp"

#include "toast_lib/io/filesystem.hpp"

#include <mono/jit/jit.h>

namespace toaster
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

		auto getRootDomain() const -> MonoDomain *;
		auto getAppDomain() const -> MonoDomain *;
		auto getAssembly() const -> MonoAssembly *;

		auto loadAssembly(const io::filesystem::Path &p_path) const -> MonoAssembly *;

		auto printAssemblyTypes(MonoAssembly *p_assembly) const -> void;

	private:
		ScriptEngineSpecInfo m_specInfo{};

		MonoDomain *m_rootDomain{nullptr};
		MonoDomain *m_appDomain{nullptr};

		MonoAssembly *m_assembly{nullptr};
	};
}
