#include "compute_pass.hpp"

namespace tst
{
	ComputePass::ComputePass(const ComputePassSpecInfo &spec)
		: m_Specification(spec)
	{
		TST_ASSERT(spec.Pipeline);

		DescriptorSetManagerSpecification dmSpec;
		dmSpec.debugName       = spec.DebugName;
		dmSpec.shader          = spec.Pipeline->getShader().as<VulkanShader>();
		dmSpec.startSet        = 1;
		m_DescriptorSetManager = DescriptorSetManager(dmSpec);
	}

	bool ComputePass::IsInvalidated(uint32_t set, uint32_t binding) const
	{
		return m_DescriptorSetManager.IsInvalidated(set, binding);
	}

	void ComputePass::SetInput(std::string_view name, RefPtr<UniformBufferSet> uniformBufferSet)
	{
		m_DescriptorSetManager.SetInput(name, uniformBufferSet);
	}

	void ComputePass::SetInput(std::string_view name, RefPtr<UniformBuffer> uniformBuffer)
	{
		m_DescriptorSetManager.SetInput(name, uniformBuffer);
	}

	void ComputePass::SetInput(std::string_view name, RefPtr<StorageBufferSet> storageBufferSet)
	{
		m_DescriptorSetManager.SetInput(name, storageBufferSet);
	}

	void ComputePass::SetInput(std::string_view name, RefPtr<StorageBuffer> storageBuffer)
	{
		m_DescriptorSetManager.SetInput(name, storageBuffer);
	}

	void ComputePass::SetInput(std::string_view name, RefPtr<Texture2D> texture)
	{
		m_DescriptorSetManager.SetInput(name, texture);
	}

	// void ComputePass::SetInput(std::string_view name, RefPtr<TextureCube> textureCube)
	// {
	// 	m_DescriptorSetManager.SetInput(name, textureCube);
	// }

	void ComputePass::SetInput(std::string_view name, RefPtr<Image2D> image)
	{
		m_DescriptorSetManager.SetInput(name, image);
	}

	RefPtr<Image2D> ComputePass::GetOutput(uint32_t index)
	{
		TST_ASSERT(false);
		return nullptr;
	}

	RefPtr<Image2D> ComputePass::GetDepthOutput()
	{
		TST_ASSERT(false);
		return nullptr;
	}

	bool ComputePass::HasDescriptorSets() const
	{
		return m_DescriptorSetManager.HasDescriptorSets();
	}

	uint32_t ComputePass::GetFirstSetIndex() const
	{
		return m_DescriptorSetManager.GetFirstSetIndex();
	}

	bool ComputePass::Validate()
	{
		return m_DescriptorSetManager.Validate();
	}

	void ComputePass::Bake()
	{
		m_DescriptorSetManager.Bake();
	}

	void ComputePass::Prepare()
	{
		m_DescriptorSetManager.InvalidateAndUpdate();
	}

	RefPtr<ComputePipeline> ComputePass::GetPipeline() const
	{
		return m_Specification.Pipeline;
	}

	bool ComputePass::IsInputValid(std::string_view name) const
	{
		std::string nameStr(name);
		return m_DescriptorSetManager.InputDeclarations.find(nameStr) != m_DescriptorSetManager.InputDeclarations.end();
	}

	const RenderInputDeclaration *ComputePass::GetInputDeclaration(std::string_view name) const
	{
		return m_DescriptorSetManager.GetInputDeclaration(name);
	}
}
