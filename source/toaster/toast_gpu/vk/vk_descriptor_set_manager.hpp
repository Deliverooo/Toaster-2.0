#pragma once

#include "vk_shader.hpp"
#include "vk_texture.hpp"
#include "vk_uniform_buffer.hpp"

namespace toaster::gpu
{


	template<typename Type>
	using PerFrameInFlight = std::array<Type, 3u>;

	class VKDescriptorSetManager
	{
	public:
		VKDescriptorSetManager(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, uint32 p_start_set, uint32 p_end_set);

		void set(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer);
		void set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d);

	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};

		vk::DescriptorPool                                m_descriptorPool{nullptr};
		PerFrameInFlight<std::vector<vk::DescriptorSet> > m_descriptorSets;

		// Set -> Binding -> Descriptor info
		PerFrameInFlight<std::unordered_map<uint32, std::unordered_map<uint32, vk::WriteDescriptorSet> > > m_writeDescriptorMap;
	};
}
