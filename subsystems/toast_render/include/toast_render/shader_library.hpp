#pragma once

#include "toast_render.hpp"

#include "toast_gpu/vk/vk_shader.hpp"

namespace toaster
{
	class TST_RENDER_API ShaderLibrary
	{
	public:
		ShaderLibrary()  = default;
		~ShaderLibrary() = default;

		auto add(const String &p_name, const gpu::ShaderHandle &p_shader) -> void;

		[[nodiscard]] auto get(const String &p_name) const -> gpu::ShaderHandle;

	private:
		std::unordered_map<String, gpu::ShaderHandle> m_shaders;
	};
}
