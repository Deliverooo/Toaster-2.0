#pragma once

#include "vk_shader.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKMaterial
	{
	public:
		VKMaterial(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader);

		template<typename Type>
		void set(const String &p_name, const Type &p_value)
		{

		}

	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};
	};
}
