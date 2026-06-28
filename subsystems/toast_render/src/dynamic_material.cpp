#include "toast_render/dynamic_material.hpp"

namespace toaster::render
{
	auto findMaterialDeclaration(const reflection::ReflectionData &p_reflection_data) -> const reflection::ReflectedStruct *
	{
		auto it{
			std::ranges::find_if(p_reflection_data.structs, [](const auto &pair) -> bool
			{
				if (pair.first.starts_with("TST__"))
					return true;
				return false;
			})
		};

		if (it != p_reflection_data.structs.end())
			return &it->second;

		LOG_ERROR("Failed to find toaster material declaration!");
		return nullptr;
	}

	DynamicMaterial::DynamicMaterial(RenderContext &                    p_render_ctx, const String &p_name,
									 const reflection::ReflectedStruct &p_material_struct_declaration) : m_renderCtx(&p_render_ctx), m_name(p_name),
																										 m_materialDeclaration(p_material_struct_declaration)
	{
		TST_PERMA_ASSERT(!m_materialDeclaration.members.empty());

		m_internalMaterialStructName = m_materialDeclaration.name.substr(std::strlen("TST__"));
		LOG_INFO("Creating material: {} | Struct: {}", m_name, m_internalMaterialStructName);

		const uint64 ubo_size{m_materialDeclaration.size};
		m_uniformData       = m_renderCtx->createUnique<UniformBuffer>(ubo_size);
		m_mappedUniformData = m_uniformData->getBuffer()->mapMemory(ubo_size);
	}

	DynamicMaterial::~DynamicMaterial()
	{
		m_uniformData->getBuffer()->unmapMemory();
	}

	auto DynamicMaterial::getDeviceAddress() const -> uintptr
	{
		return m_uniformData->getDeviceAddress();
	}

	auto DynamicMaterial::_set(const String &p_name, const void *p_value) -> void
	{
		auto it{m_materialDeclaration.members.find(p_name)};
		if (it == m_materialDeclaration.members.end())
		{
			LOG_ERROR("Material '{}' does not contain member '{}' | struct: {}", m_name, p_name, m_internalMaterialStructName);
			return;
		}
		uintptr offset{(uintptr) it->second.offset};
		uint32  size{it->second.size};
		std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uintptr>(m_mappedUniformData) + offset), p_value, size);
	}
}
