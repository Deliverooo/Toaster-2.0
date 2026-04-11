#include "vk_material.hpp"

namespace toaster::gpu
{
	VKMaterial::VKMaterial(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader) : m_ctx(p_ctx), m_shader(p_shader)
	{
		const auto &push_constant_buffers{m_shader->getReflectedPushConstantBuffers()};
		if (push_constant_buffers.size() > 0)
		{
			uint32 size{0u};
			for (auto [name, push_constant]: push_constant_buffers)
				size += push_constant.size;

			m_pushConstantStorageBuffer.allocate(size);
			m_pushConstantStorageBuffer.zeroInitialize();
		}

		m_descriptorSetManager = make_unique<VKDescriptorSetManager>(m_ctx, m_shader, 0, 0);

		for (const auto &[name, decl]: m_descriptorSetManager->getDescriptorDeclarations())
		{
			switch (decl.type)
			{
				case EDescriptorType::eSampler2D:
				{
					for (uint32 i{0u}; i < decl.arraySize; ++i)
						m_descriptorSetManager->setDescriptor(name, m_descriptorSetManager->getWhiteTexture(), i);
					break;
				}
				default: break;
			}
		}
		m_descriptorSetManager->bakeDescriptors();
	}

	VKMaterial::~VKMaterial()
	{
		m_pushConstantStorageBuffer.release();
	}

	VKGPUContext *VKMaterial::getContext() const
	{
		return m_ctx;
	}

	void VKMaterial::set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d)
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	void VKMaterial::set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index)
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d, p_array_index);
	}

	void VKMaterial::update(uint32 p_frame_index)
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	vk::DescriptorSet VKMaterial::getDescriptorSet(uint32 p_frame_index)
	{
		update(p_frame_index);
		return m_descriptorSetManager->getDescriptorSets(p_frame_index)[0];
	}

	bool VKMaterial::hasDescriptorSets() const
	{
		return m_descriptorSetManager->hasDescriptorSets();
	}

	const Buffer &VKMaterial::getPushConstantStorageBuffer() const
	{
		return m_pushConstantStorageBuffer;
	}

	const PushConstant *VKMaterial::_getPushConstantDeclaration(const String &p_name)
	{
		const auto &push_constant_buffers{m_shader->getReflectedPushConstantBuffers()};
		if (push_constant_buffers.size() > 0)
		{
			const PushConstantBuffer &push_constant_buffer{(push_constant_buffers.begin())->second};
			if (!push_constant_buffer.pushConstants.contains(p_name))
				return nullptr;

			return &push_constant_buffer.pushConstants.at(p_name);
		}
		return nullptr;
	}
}
