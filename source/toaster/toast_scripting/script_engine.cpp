#include "script_engine.hpp"

#include <Windows.h>
#include <mono/metadata/assembly.h>

#include "toast_lib/logging.hpp"
#include "toast_lib/toast_assert.h"
#include "toast_lib/os/file_dialog.hpp"
#include "toast_lib/os/library_loading.hpp"

namespace toaster::script
{
	ScriptEngine::ScriptEngine(const ScriptEngineSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		mono_set_dirs("C:/Program Files/Mono/lib", "C:/Program Files/Mono/etc");

		m_rootDomain = mono_jit_init_version(m_specInfo.rootDomainName.c_str(), "v4.0.30319");

		m_appDomain = mono_domain_create_appdomain(const_cast<char *>(m_specInfo.appDomainName.c_str()), nullptr);
		mono_domain_set(m_appDomain, true);

		m_coreAssembly = loadAssembly(m_specInfo.coreAssemblyPath);
		m_appAssembly  = loadAssembly(m_specInfo.appAssemblyPath);
		TST_PERMA_ASSERT_MSG(m_appAssembly, "Failed to load core assembly");
		m_coreImage = mono_assembly_get_image(m_coreAssembly);
		m_appImage  = mono_assembly_get_image(m_appAssembly);
	}

	ScriptEngine::~ScriptEngine()
	{
		mono_domain_set(m_rootDomain, true);
		mono_domain_unload(m_appDomain);

		mono_jit_cleanup(m_rootDomain);
	}

	auto ScriptEngine::getSpecInfo() const -> const ScriptEngineSpecInfo &
	{
		return m_specInfo;
	}

	auto ScriptEngine::getRootDomain() const -> MonoDomain *
	{
		return m_rootDomain;
	}

	auto ScriptEngine::getAppDomain() const -> MonoDomain *
	{
		return m_appDomain;
	}

	auto ScriptEngine::getCoreAssembly() const -> MonoAssembly *
	{
		return m_coreAssembly;
	}

	auto ScriptEngine::getCoreImage() const -> MonoImage *
	{
		return m_coreImage;
	}

	auto ScriptEngine::getAppAssembly() const -> MonoAssembly *
	{
		return m_appAssembly;
	}

	auto ScriptEngine::getAppImage() const -> MonoImage *
	{
		return m_appImage;
	}

	auto ScriptEngine::loadAssembly(const io::filesystem::Path &p_path) const -> MonoAssembly *
	{
		MonoImageOpenStatus status{};
		MonoAssembly *      assembly{mono_assembly_open(p_path.string().c_str(), &status)};

		if (status != MONO_IMAGE_OK)
		{
			LOG_ERROR("Failed to load assembly: {}", p_path.string());
			return nullptr;
		}

		return assembly;
	}

	auto ScriptEngine::printAssemblyTypes(MonoAssembly *p_assembly) const -> void
	{
		MonoImage *          image{mono_assembly_get_image(p_assembly)};
		const MonoTableInfo *type_definitions{mono_image_get_table_info(image, MONO_TABLE_TYPEDEF)};

		MonoClass *entity_class{mono_class_from_name(image, "Toaster", "Entity")};

		for (uint32 row{0u}; row < mono_table_info_get_rows(type_definitions); ++row)
		{
			uint32 cols[MONO_TYPEDEF_SIZE]{};
			mono_metadata_decode_row(type_definitions, row, cols, MONO_TYPEDEF_SIZE);

			auto name_space{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE])};
			auto type_name{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME])};

			MonoClass *script_class{mono_class_from_name(image, name_space, type_name)};
			if (mono_class_is_subclass_of(entity_class, script_class, false))
			{
				LOG_ERROR("Is entity");
			}
			auto method_name{mono_metadata_string_heap(image, cols[MONO_TYPEDEF_METHOD_LIST])};
			LOG_INFO("Namespace: {} | Type: {} | Method: {}", name_space, type_name, method_name);
		}
	}
}
