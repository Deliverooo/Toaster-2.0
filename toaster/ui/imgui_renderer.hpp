#pragma once
#include <map>
#include <unordered_map>

#include "imgui.h"
#include "gpu/command_buffer.hpp"
#include "gpu/vulkan/vulkan_swapchain.hpp"

namespace tst
{
	class ImGuiRenderer
	{
	public:
		ImGuiRenderer() = default;

		bool init();
		bool updateFontTexture();
		bool render(ImGuiViewport *viewport, const nvrhi::GraphicsPipelineHandle &pipeline, const nvrhi::FramebufferHandle &framebuffer,
					vk::Semaphore  waitSemaphore = nullptr);
		bool renderToSwapchain(ImGuiViewport *viewport, VulkanSwapChain *swapchain);
		void backbufferResizing();

	private:
		bool reallocateBuffer(nvrhi::BufferHandle &buffer, size_t requiredSize, size_t reallocateSize, bool isIndexBuffer);

		nvrhi::GraphicsPipelineHandle getOrCreatePipeline(VulkanSwapChain *swapchain);

		nvrhi::IBindingSet *getBindingSet(nvrhi::ITexture *texture);
		bool                updateGeometry(ImDrawData *drawData);

		RefPtr<CommandBuffer> m_RenderCommandBuffer;

		nvrhi::ShaderHandle      m_VertexShader;
		nvrhi::ShaderHandle      m_PixelShader;
		nvrhi::InputLayoutHandle m_ShaderAttribLayout;

		nvrhi::TextureHandle m_FontTexture;
		nvrhi::SamplerHandle m_FontSampler;

		nvrhi::BufferHandle m_VertexBuffer;
		nvrhi::BufferHandle m_IndexBuffer;

		nvrhi::BindingLayoutHandle  m_BindingLayout;
		nvrhi::GraphicsPipelineDesc m_BasePSODesc;

		std::unordered_map<nvrhi::ITexture *, nvrhi::BindingSetHandle> m_BindingsCache;

		std::vector<ImDrawVert> m_VertexBufferData;
		std::vector<ImDrawIdx>  m_IndexBufferData;

		struct SwapchainPipelineCache
		{
			std::array<nvrhi::FramebufferHandle, 3>      Framebuffers;
			std::array<nvrhi::GraphicsPipelineHandle, 3> Pipelines;
		};

		std::map<VulkanSwapChain *, SwapchainPipelineCache> m_PipelineCache;
	};
}
