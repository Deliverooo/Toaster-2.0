#pragma once

#include "vk_shader.hpp"
#include "../buffer_layout.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct PipelineCreateInfo
	{
		VertexBufferLayout vertexBufferLayout;
		InstanceLayout     instanceLayout;

		// Thanks to Vulkan 1.3's dynamic rendering, when creating a pipeline, you only need to specify the formats for your rendering attachments
		// This is very helpful because I hate framebuffers and Vulkan render passes.
		std::vector<vk::Format> colourAttachments{};
		vk::Format              depthFormat{vk::Format::eUndefined};

		RefPtr<VKShader> shader{nullptr};

		vk::PrimitiveTopology primitiveTopology{vk::PrimitiveTopology::eTriangleList};
		vk::PolygonMode       polygonMode{vk::PolygonMode::eFill};
		vk::CullModeFlags     cullMode{vk::CullModeFlagBits::eBack};

		bool multisample{false}; // This should not be enabled by default
	};

	class VKPipeline
	{
	public:
		VKPipeline(VKGPUContext *p_ctx, const PipelineCreateInfo &p_create_info);
		auto getContext() const -> VKGPUContext *;

		[[nodiscard]] auto getPipeline() -> vk::raii::Pipeline &;
		[[nodiscard]] auto getPipelineLayout() -> vk::raii::PipelineLayout &;

		[[nodiscard]] auto getCreateInfo() const -> const PipelineCreateInfo &;

	private:
		auto _createGraphicsPipeline() -> void;

		auto _getVulkanAttribType(EBufferDataType p_type) -> vk::Format;

		VKGPUContext *m_ctx{nullptr};

		PipelineCreateInfo m_createInfo{};

		vk::raii::Pipeline       m_graphicsPipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};
}
