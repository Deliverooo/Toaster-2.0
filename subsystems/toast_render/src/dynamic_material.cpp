#include "toast_render/dynamic_material.hpp"

namespace toaster::render
{
	NDynamicMaterial::NDynamicMaterial(const String &p_name) : m_name(p_name)
	{
	}

	auto NDynamicMaterial::set(const String &p_name, const ImageHandle &p_image) -> void
	{
		m_imageRefs[p_name] = p_image;
	}
}
