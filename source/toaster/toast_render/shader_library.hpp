#pragma once

#include "../toaster_export.hpp"

#include <unordered_map>

#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster
{
	class TST_API ShaderLibrary
	{
	public:
		ShaderLibrary()  = default;
		~ShaderLibrary() = default;

		auto add(const String &p_name, const RefPtr<gpu::VKShader> &p_shader) -> void;

		[[nodiscard]] auto get(const String &p_name) const -> RefPtr<gpu::VKShader>;

	private:
		std::unordered_map<String, RefPtr<gpu::VKShader> > m_shaders;
	};
}
