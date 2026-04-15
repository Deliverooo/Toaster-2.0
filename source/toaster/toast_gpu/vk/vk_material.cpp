#include "vk_material.hpp"

#include <ranges>

namespace toaster::gpu
{
	VKMaterial::VKMaterial(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, const String &p_name) : m_ctx(p_ctx), m_shader(p_shader), m_name(p_name)
	{
		const auto &push_constant_buffers{m_shader->getReflectedPushConstantBuffers()};
		if (!push_constant_buffers.empty())
		{
			uint32 size{0u};
			for (const auto &push_constant: push_constant_buffers | std::views::values)
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

	auto VKMaterial::getContext() const -> VKGPUContext *
	{
		return m_ctx;
	}

	auto VKMaterial::set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	auto VKMaterial::set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d, p_array_index);
	}

	auto VKMaterial::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto VKMaterial::getDescriptorSet(uint32 p_frame_index) -> vk::DescriptorSet
	{
		update(p_frame_index);
		return m_descriptorSetManager->getDescriptorSets(p_frame_index)[0];
	}

	auto VKMaterial::hasDescriptorSets() const -> bool
	{
		return m_descriptorSetManager->hasDescriptorSets();
	}

	auto VKMaterial::getPushConstantStorageBuffer() const -> const Buffer &
	{
		return m_pushConstantStorageBuffer;
	}

	auto VKMaterial::getName() const -> String
	{
		return m_name;
	}

	auto VKMaterial::_getPushConstantDeclaration(const String &p_name) -> const PushConstant *
	{
		const auto &push_constant_buffers{m_shader->getReflectedPushConstantBuffers()};
		if (!push_constant_buffers.empty())
		{
			for (const auto &[name, pcb]: push_constant_buffers)
			{
				if (pcb.pushConstants.contains(p_name))
					return &push_constant_buffers.at(name).pushConstants.at(p_name);
			}
		}
		TST_ASSERT(false);
		return nullptr;
	}
}
