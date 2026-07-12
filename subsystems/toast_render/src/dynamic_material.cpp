#include "toast_render/dynamic_material.hpp"
#include "toast_render/render_context.hpp"

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

		// LOG_ERROR("Failed to find toaster material declaration!");
		return nullptr;
	}

	DynamicMaterial::DynamicMaterial(RenderContext &                    p_render_ctx, const gpu::ShaderHandle &p_vertex_shader, const gpu::ShaderHandle &p_pixel_shader,
									 const reflection::ReflectedStruct *p_material_struct_declaration, const String &p_name) : m_renderCtx(&p_render_ctx),
																															   m_vertexShader(p_vertex_shader),
																															   m_pixelShader(p_pixel_shader),
																															   m_name(p_name)
	{
		if (!p_material_struct_declaration)
		{
			auto reflection_data{reflection::reflectShader(*m_pixelShader)};
			auto material_decl{findMaterialDeclaration(reflection_data)};
			TST_PERMA_ASSERT_MSG(material_decl, "Could not find a suitable material declaration in the provided shader!!");
			m_materialDeclaration = *material_decl;
		}

		m_materialDeclaration = *p_material_struct_declaration;
		TST_PERMA_ASSERT(!m_materialDeclaration.members.empty());

		m_internalMaterialStructName = m_materialDeclaration.name.substr(std::strlen("TST__"));
		LOG_INFO("Creating material: {} | Struct: {}", m_name, m_internalMaterialStructName);

		const uint64 ubo_size{m_materialDeclaration.size};
		m_uniformData       = m_renderCtx->createGPUUnique<gpu::UniformBuffer>(ubo_size);
		m_mappedUniformData = m_uniformData->mapMemory(ubo_size);
	}

	DynamicMaterial::~DynamicMaterial()
	{
		m_uniformData->unmapMemory();
	}

	auto DynamicMaterial::getVertexShader() const -> const gpu::ShaderHandle &
	{
		return m_vertexShader;
	}

	auto DynamicMaterial::getPixelShader() const -> const gpu::ShaderHandle &
	{
		return m_pixelShader;
	}

	auto DynamicMaterial::getDeviceAddress() const -> uintptr
	{
		return m_uniformData->getDeviceAddress();
	}

	auto DynamicMaterial::getHeapID() const -> uint32
	{
		return m_uniformData->getHeapID();
	}

	auto DynamicMaterial::getImage(const String &p_name) const -> const gpu::ImageHandle &
	{
		return m_imageRefs.at(p_name);
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
