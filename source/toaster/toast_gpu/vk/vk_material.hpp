#pragma once

#include "vk_shader.hpp"
#include "toast_lib/ptr.hpp"

namespace toaster::gpu
{
	class VKMaterial
	{
	public:

	private:
		RefPtr<VKShader> m_shader{nullptr};
	};
}
