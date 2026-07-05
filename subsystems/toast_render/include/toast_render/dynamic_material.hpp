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

		[[nodiscard]] auto getDeviceAddress() const -> uintptr;

		template<typename Type>
		auto set(const String &p_name, const Type &p_value) -> void
		{
			_set(p_name, static_cast<const void *>(&p_value));
		}

		auto getImage(const String &p_name) const -> const ImageHandle &;

		uint64 flags{0ull};

	private:
		auto _set(const String &p_name, const void *p_value) -> void;

		String m_name;

		String                      m_internalMaterialStructName;
		reflection::ReflectedStruct m_materialDeclaration;

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
