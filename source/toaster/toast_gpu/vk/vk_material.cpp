#include "vk_material.hpp"


namespace toaster::gpu
{
	VKMaterial::VKMaterial(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader) : m_ctx(p_ctx), m_shader(p_shader)
	{
		TST_ASSERT_MSG(p_ctx, "Context cannot be null");

		auto &shader_resources = m_shader->getReflectedShaderResources();
		for (auto &[name, resource] : shader_resources)
		{
			LOG_WARN("Resource: {}", name);
		}
	}
}
