#pragma once

#include "vk_descriptor_set_manager.hpp"
#include "vk_shader.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKMaterial
	{
	public:
		VKMaterial(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader);

		void set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d);

		void update(uint32 p_frame_index);

		vk::DescriptorSet getDescriptorSet(uint32 p_frame_index);
	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};

		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};
	};
}
