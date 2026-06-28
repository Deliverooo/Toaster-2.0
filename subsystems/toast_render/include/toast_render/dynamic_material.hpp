#pragma once

#include "image.hpp"
#include "toast_render.hpp"
#include "uniform_buffer.hpp"

#include "shader_reflection.hpp"

namespace toaster::render
{
	[[nodiscard]] TST_RENDER_API auto findMaterialDeclaration(const reflection::ReflectionData &p_reflection_data) -> const reflection::ReflectedStruct *;

	class TST_RENDER_API DynamicMaterial
	{
		TST_RENDER_OBJECT
	public:
		DynamicMaterial(RenderContext &p_render_ctx, const String &p_name, const reflection::ReflectedStruct &p_material_struct_declaration);
		~DynamicMaterial();

		auto getDeviceAddress() const -> uintptr;

		auto set(const String &p_name, const ImageHandle &p_image) -> void;
		auto set(const String &p_name, uint32 p_val) -> void;

	private:
		String m_name;

		String                      m_internalMaterialStructName;
		reflection::ReflectedStruct m_materialDeclaration;

		std::unordered_map<String, ImageHandle> m_imageRefs;

		// The buffer containing the data stored in the shader ubo
		UniformBufferUnique m_uniformData{nullptr};
		void *              m_mappedUniformData{nullptr};
	};

	TST_RENDER_DEFINE_HANDLE(DynamicMaterial, DynamicMaterial);
}
