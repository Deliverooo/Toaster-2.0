#pragma once

#include "../toaster_macros.hpp"

#include "toast_lib/io/filesystem.hpp"

#include <mono/jit/jit.h>

#include "toast_lib/ptr.hpp"

namespace toaster::script
{
	constexpr auto c_scriptConfigProfile{"Debug"}; // TODO: Release / debug script things...

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
}
