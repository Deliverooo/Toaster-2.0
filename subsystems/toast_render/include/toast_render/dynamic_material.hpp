#pragma once

#include "image.hpp"
#include "toast_render.hpp"
#include "uniform_buffer.hpp"

#include "shader_reflection.hpp"

namespace toaster::render
{
	[[nodiscard]] TST_RENDER_API auto findMaterialDeclaration(const reflection::ReflectionData &p_reflection_data) -> const reflection::ReflectedStruct *;

	enum EMaterialPropertyFlags : uint32
	{
		eNone        = 0u,
		eTwoSided    = BIT(0u),
		eWireframe   = BIT(1u),
		eTransparent = BIT(2u)
	};

	class TST_RENDER_API DynamicMaterial
	{
		TST_RENDER_OBJECT
	public:
		// The shader to pass should be a pixel shader... Actually, technically it doesn't even matter
		DynamicMaterial(RenderContext &p_render_ctx, const gpu::ShaderHandle &p_shader, const reflection::ReflectedStruct *p_material_struct_declaration,
						const String & p_name = "New_Material");
		~DynamicMaterial();

		[[nodiscard]] auto getShader() const -> const gpu::ShaderHandle &;

		[[nodiscard]] auto getDeviceAddress() const -> uintptr;

		template<typename Type>
		auto set(const String &p_name, const Type &p_value) -> void
		{
			_set(p_name, static_cast<const void *>(&p_value));
		}

		[[nodiscard]] auto getImage(const String &p_name) const -> const ImageHandle &;

		uint32 flags{EMaterialPropertyFlags::eNone};

	private:
		auto _set(const String &p_name, const void *p_value) -> void;

		gpu::ShaderHandle           m_shader{nullptr};
		reflection::ReflectedStruct m_materialDeclaration;

		String m_name;
		String m_internalMaterialStructName;

		std::unordered_map<String, ImageHandle> m_imageRefs;

		// The buffer containing the data stored in the shader ubo
		UniformBufferUnique m_uniformData{nullptr};
		void *              m_mappedUniformData{nullptr};
	};

	template<>
	inline auto DynamicMaterial::set<ImageHandle>(const String &p_name, const ImageHandle &p_value) -> void
	{
		uint32 image_heap_id{p_value->getAlignedShaderReadHeapID()};
		_set(p_name, &image_heap_id);
		m_imageRefs[p_name] = p_value;
	}

	TST_RENDER_DEFINE_HANDLE(DynamicMaterial, DynamicMaterial);
}
