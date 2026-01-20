#pragma once

#include "shader.hpp"

namespace toaster::gpu
{
	class Material
	{
	public:
		static Material create(const RefPtr<Shader> &p_shader);
		virtual         ~Material() = default;

		virtual void bind();
		virtual void unbind();

	private:
		RefPtr<Shader> m_shader;
	};
}
