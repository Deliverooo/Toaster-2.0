#pragma once

#include "vk_shader.hpp"
#include "vk_uniform_buffer.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	enum class EDescriptorType
	{
		eUnknown, eUniformBuffer
	};

	struct DescriptorDeclaration
	{
		String          name{};
		uint32          set{0u};
		uint32          binding{0u};
		uint32          arraySize{0u};
		EDescriptorType type{EDescriptorType::eUnknown};
	};

	class VKDescriptorSetManager
	{
	public:
		VKDescriptorSetManager(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, uint32 p_start_set, uint32 p_end_set);

		void setDescriptor(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer);
		void setDescriptor(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff);

		// Only call when you have set all your required descriptors :)
		void bakeDescriptors();

		[[nodiscard]] const std::vector<vk::raii::DescriptorSet> &getDescriptorSets(uint32 p_frame_index) const;

	private:
		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};
		uint32           m_startSet{0u};
		uint32           m_endSet{3u};

		vk::raii::DescriptorPool m_descriptorPool{nullptr};

		struct WriteDescriptor
		{
			vk::WriteDescriptorSet wds{};
			std::vector<void *>    resourceHandles;
		};

		std::vector<std::unordered_map<uint32, std::unordered_map<uint32, WriteDescriptor> > > m_writeDescriptorMap;
		std::unordered_map<String, DescriptorDeclaration> m_descriptorDeclarations;
	};
}
