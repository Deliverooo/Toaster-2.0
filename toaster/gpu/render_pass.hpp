#pragma once

#include "pipeline.hpp"
#include "vulkan/descriptor_set_manager.hpp"

namespace tst
{
	struct RenderPassSpecInfo
	{
		RefPtr<Pipeline> pipeline;
		std::string      debugName;
	};

	class RenderPass : public RefCounted
	{
	public:
		RenderPassSpecInfo &      getSpecInfo() { return m_specInfo; }
		const RenderPassSpecInfo &getSpecInfo() const { return m_specInfo; }

		void setInput(std::string_view name, RefPtr<UniformBufferSet> uniformBufferSet);
		void setInput(std::string_view name, RefPtr<UniformBuffer> uniformBuffer);

		void setInput(std::string_view name, RefPtr<StorageBufferSet> storageBufferSet);
		void setInput(std::string_view name, RefPtr<StorageBuffer> storageBuffer);

		void setInput(std::string_view name, RefPtr<Texture2D> texture);
		// void setInput(std::string_view name, RefPtr<TextureCube> textureCube);
		void setInput(std::string_view name, RefPtr<Image2D> image);
		void setInput(std::string_view name, RefPtr<Sampler> sampler);

		RefPtr<Image2D> getOutput(uint32_t index);
		RefPtr<Image2D> getDepthOutput();
		uint32_t        getFirstSetIndex() const;

		RefPtr<Framebuffer> getTargetFramebuffer() const;
		RefPtr<Pipeline>    getPipeline() const;

		bool validate();
		void bake();
		void prepare();

		bool                    hasDescriptorSets() const;
		nvrhi::BindingSetVector getBindingSets(uint32_t frameIndex) const { return m_descriptorSetManager.GetBindingSets(frameIndex); }

		bool                          isInputValid(std::string_view name) const;
		const RenderInputDeclaration *getInputDeclaration(std::string_view name) const;

		RenderPass(const RenderPassSpecInfo &spec);
		virtual ~RenderPass() = default;

	private:
		bool isInvalidated(uint32_t set, uint32_t binding) const;

		RenderPassSpecInfo   m_specInfo;
		DescriptorSetManager m_descriptorSetManager;
	};
}
