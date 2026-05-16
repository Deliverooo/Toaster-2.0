#include "material.hpp"
#include <ranges>
#include "render_context.hpp"

namespace toaster::render
{
	Material::Material(RenderContext *p_render_ctx, const gpu::ShaderHandle &p_shader, const String &p_name) : m_renderCtx(p_render_ctx), m_shader(p_shader),
																											   m_name(p_name)
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

		m_descriptorSetManager = new gpu::VKDescriptorSetManager{m_renderCtx->getLogicalDevice(), m_shader, 0, 0};

		for (const auto &[name, decl]: m_descriptorSetManager->getDescriptorDeclarations())
		{
			switch (decl.type)
			{
				case gpu::EDescriptorType::eSampler2D:
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

	Material::~Material()
	{
		m_pushConstantStorageBuffer.release();

		m_renderCtx->getLogicalDevice()->deferDestruction([dsm = m_descriptorSetManager]()mutable -> void
		{
			delete dsm;
		});
	}

	auto Material::setTexture(const String &p_name, const gpu::Texture2DHandle &p_texture_2d) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d);
	}

	auto Material::setTexture(const String &p_name, const gpu::Texture2DHandle &p_texture_2d, uint32 p_array_index) -> void
	{
		m_descriptorSetManager->setDescriptor(p_name, p_texture_2d, p_array_index);
	}

	auto Material::update(uint32 p_frame_index) -> void
	{
		m_descriptorSetManager->updateDescriptors(p_frame_index);
	}

	auto Material::getDescriptorSet(uint32 p_frame_index) -> vk::DescriptorSet
	{
		update(p_frame_index);
		return m_descriptorSetManager->getDescriptorSets(p_frame_index)[0];
	}

	auto Material::hasDescriptorSets() const -> bool
	{
		return m_descriptorSetManager->hasDescriptorSets();
	}

	auto Material::getPushConstantStorageBuffer() const -> const Buffer &
	{
		return m_pushConstantStorageBuffer;
	}

	auto Material::getName() const -> String
	{
		return m_name;
	}

	auto Material::_getPushConstantDeclaration(const String &p_name) -> const gpu::PushConstant *
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
		TST_ASSERT_MSG(false, "Failed to find push constant definition");
		return nullptr;
	}
}
