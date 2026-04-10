#pragma once

#include <unordered_map>

#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster
{
	class ShaderLibrary
	{
	public:
		ShaderLibrary()  = default;
		~ShaderLibrary() = default;

		void add(const std::string &p_name, const RefPtr<gpu::VKShader> &p_shader);

		[[nodiscard]] RefPtr<gpu::VKShader> get(const std::string &p_name) const;

	private:
		std::unordered_map<std::string, RefPtr<gpu::VKShader> > m_shaders;
	};
}
