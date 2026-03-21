#pragma once

#include <unordered_map>

#include "toaster/toast_gpu/shader.hpp"

namespace toaster
{
	class ShaderLibrary
	{
	public:
		ShaderLibrary()  = default;
		~ShaderLibrary() = default;

		void add(const std::string &p_name, const RefPtr<gpu::IShader> &p_shader);

		[[nodiscard]] RefPtr<gpu::IShader> get(const std::string &p_name) const;

	private:
		std::unordered_map<std::string, RefPtr<gpu::IShader> > m_shaders;
	};
}
