#pragma once

#include "vulkan/descriptor_set_manager.hpp"
#include "gpu/compute_pipeline.hpp"

namespace tst
{
	struct ComputePassSpecInfo
	{
		RefPtr<ComputePipeline> Pipeline;
		std::string             DebugName;
	};

	class ComputePass : public RefCounted
	{
	public:
		ComputePassSpecInfo &      GetSpecification() { return m_Specification; }
		const ComputePassSpecInfo &GetSpecification() const { return m_Specification; }

		RefPtr<Shader> GetShader() const { return m_Specification.Pipeline->getShader(); }

		void SetInput(std::string_view name, RefPtr<UniformBufferSet> uniformBufferSet);
		void SetInput(std::string_view name, RefPtr<UniformBuffer> uniformBuffer);

		void SetInput(std::string_view name, RefPtr<StorageBufferSet> storageBufferSet);
		void SetInput(std::string_view name, RefPtr<StorageBuffer> storageBuffer);

		void SetInput(std::string_view name, RefPtr<Texture2D> texture);
		// void SetInput(std::string_view name, RefPtr<TextureCube> textureCube);
		void SetInput(std::string_view name, RefPtr<Image2D> image);

		RefPtr<Image2D> GetOutput(uint32_t index);
		RefPtr<Image2D> GetDepthOutput();
		bool            HasDescriptorSets() const;
		uint32_t        GetFirstSetIndex() const;

		bool Validate();
		void Bake();
		void Prepare();

		const nvrhi::BindingSetVector &GetBindingSets(uint32_t frameIndex) const { return m_DescriptorSetManager.GetBindingSets(frameIndex); }

		virtual RefPtr<ComputePipeline> GetPipeline() const;

		bool                          IsInputValid(std::string_view name) const;
		const RenderInputDeclaration *GetInputDeclaration(std::string_view name) const;

		ComputePass(const ComputePassSpecInfo &spec);
		virtual ~ComputePass() = default;

	private:
		bool IsInvalidated(uint32_t set, uint32_t binding) const;

	private:
		ComputePassSpecInfo m_Specification;
		DescriptorSetManager     m_DescriptorSetManager;
	};
}
