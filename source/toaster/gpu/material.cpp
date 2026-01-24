#include "material.hpp"

#include "gl/gl_material.hpp"

namespace toaster::gpu
{
	RefPtr<Material> Material::create(const RefPtr<Shader> &p_shader, const std::string &p_name)
	{
		return std::make_shared<GLMaterial>(p_shader, p_name);
	}
}
