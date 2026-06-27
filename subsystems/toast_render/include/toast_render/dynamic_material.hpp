#pragma once

#include "image.hpp"
#include "toast_render.hpp"
#include "uniform_buffer.hpp"

namespace toaster::render
{

	class TST_RENDER_API NDynamicMaterial
	{
		TST_RENDER_OBJECT
	public:
		NDynamicMaterial(const String& p_name);

		auto set(const String &p_name, const ImageHandle &p_image) -> void;

	private:
		String m_name;

		std::unordered_map<String, ImageHandle> m_imageRefs;

		// The buffer containing the data stored in the shader ubo
		UniformBufferUnique m_uniformData{nullptr};
	};
}
