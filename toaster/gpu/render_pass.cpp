#include "gpu/render_pass.hpp"

#include "renderer/renderer.hpp"

namespace tst
{
	RenderPass::RenderPass(const RenderPassSpecInfo &spec)
		: m_specInfo(spec)
	{
		TST_ASSERT(spec.pipeline);

		DescriptorSetManagerSpecification dmSpec;
		dmSpec.debugName       = spec.debugName;
		dmSpec.shader          = spec.pipeline->getSpecInfo().shader.as<VulkanShader>();
		dmSpec.startSet        = 1;
		m_descriptorSetManager = DescriptorSetManager(dmSpec);
	}

	bool RenderPass::isInvalidated(uint32_t set, uint32_t binding) const
	{
		return m_descriptorSetManager.IsInvalidated(set, binding);
	}

	void RenderPass::setInput(std::string_view name, RefPtr<UniformBufferSet> uniformBufferSet)
	{
		m_descriptorSetManager.SetInput(name, uniformBufferSet);
	}

	void RenderPass::setInput(std::string_view name, RefPtr<UniformBuffer> uniformBuffer)
	{
		m_descriptorSetManager.SetInput(name, uniformBuffer);
	}

	void RenderPass::setInput(std::string_view name, RefPtr<StorageBufferSet> storageBufferSet)
	{
		m_descriptorSetManager.SetInput(name, storageBufferSet);
	}

	void RenderPass::setInput(std::string_view name, RefPtr<StorageBuffer> storageBuffer)
	{
		m_descriptorSetManager.SetInput(name, storageBuffer);
	}

	void RenderPass::setInput(std::string_view name, RefPtr<Texture2D> texture)
	{
		m_descriptorSetManager.SetInput(name, texture);
	}

	// void RenderPass::SetInput(std::string_view name, RefPtr<TextureCube> textureCube)
	// {
	// 	m_DescriptorSetManager.SetInput(name, textureCube);
	// }

	void RenderPass::setInput(std::string_view name, RefPtr<Image2D> image)
	{
		m_descriptorSetManager.SetInput(name, image);
	}

	void RenderPass::setInput(std::string_view name, RefPtr<Sampler> sampler)
	{
		m_descriptorSetManager.SetInput(name, sampler);
	}

	RefPtr<Image2D> RenderPass::getOutput(uint32_t index)
	{
		RefPtr<Framebuffer> framebuffer = m_specInfo.pipeline->getSpecInfo().targetFramebuffer;
		if (index > framebuffer->getColorAttachmentCount() + 1)
			return nullptr; // Invalid index
		if (index < framebuffer->getColorAttachmentCount())
			return framebuffer->getImage(index);
		return framebuffer->getDepthImage();
	}

	RefPtr<Image2D> RenderPass::getDepthOutput()
	{
		RefPtr<Framebuffer> framebuffer = m_specInfo.pipeline->getSpecInfo().targetFramebuffer;
		if (!framebuffer->hasDepthAttachment())
			return nullptr; // No depth output
		return framebuffer->getDepthImage();
	}

	uint32_t RenderPass::getFirstSetIndex() const
	{
		return m_descriptorSetManager.GetFirstSetIndex();
	}

	RefPtr<Framebuffer> RenderPass::getTargetFramebuffer() const
	{
		return m_specInfo.pipeline->getSpecInfo().targetFramebuffer;
	}

	RefPtr<Pipeline> RenderPass::getPipeline() const
	{
		return m_specInfo.pipeline;
	}

	bool RenderPass::validate()
	{
		return m_descriptorSetManager.Validate();
	}

	void RenderPass::bake()
	{
		m_descriptorSetManager.Bake();
	}

	void RenderPass::prepare()
	{
		m_descriptorSetManager.InvalidateAndUpdate();
	}

	bool RenderPass::hasDescriptorSets() const
	{
		return m_descriptorSetManager.HasDescriptorSets();
	}

	bool RenderPass::isInputValid(std::string_view name) const
	{
		std::string nameStr(name);
		return m_descriptorSetManager.InputDeclarations.find(nameStr) != m_descriptorSetManager.InputDeclarations.end();
	}

	const RenderInputDeclaration *RenderPass::getInputDeclaration(std::string_view name) const
	{
		std::string nameStr(name);
		if (m_descriptorSetManager.InputDeclarations.find(nameStr) == m_descriptorSetManager.InputDeclarations.end())
			return nullptr;
		const RenderInputDeclaration &decl = m_descriptorSetManager.InputDeclarations.at(nameStr);
		return &decl;
	}
}
