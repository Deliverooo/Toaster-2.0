#pragma once

#include "toast_render.hpp"
#include "toast_gpu/vk/vk_shader.hpp"

namespace toaster::render::reflection
{
	struct ReflectedStructMember
	{
		String name;
		uint32 offset{0u};
		uint32 size{0u};
	};

	struct ReflectedStruct
	{
		String name;
		uint32 size{0u};

		std::unordered_map<String, ReflectedStructMember> members;
	};

	struct ReflectionData
	{
		std::unordered_map<String, ReflectedStruct> structs;
	};

	[[nodiscard]] TST_RENDER_API auto reflectShader(const gpu::Shader &p_shader) -> ReflectionData;
}
