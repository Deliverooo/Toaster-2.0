#pragma once

#include <unordered_map>

#include "shader.hpp"

namespace toaster
{
	class ShaderLibrary
	{
	public:
		ShaderLibrary()  = default;
		~ShaderLibrary() = default;

		void add(const std::string &p_name, const RefPtr<gpu::Shader> &p_shader);

		RefPtr<gpu::Shader> get(const std::string &p_name) const;

	private:
		std::unordered_map<std::string, RefPtr<gpu::Shader> > m_shaders;
	};
}
