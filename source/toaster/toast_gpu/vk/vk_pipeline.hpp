#pragma once

#include "vk_shader.hpp"
#include "../vertex_buffer_layout.hpp"

#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	struct PipelineCreateInfo
	{
		VertexBufferLayout vertexBufferLayout;

		// Thanks to Vulkan 1.3's dynamic rendering, when creating a pipeline, you only need to specify the formats for your rendering attachments
		// This is very helpful because I hate framebuffers and Vulkan render passes.
		std::vector<vk::Format> colourAttachments{};
		vk::Format              depthFormat{vk::Format::eUndefined};

		RefPtr<VKShader> shader{nullptr};

		vk::PrimitiveTopology primitiveTopology{vk::PrimitiveTopology::eTriangleList};
		vk::PolygonMode       polygonMode{vk::PolygonMode::eFill};
		vk::CullModeFlags     cullMode{vk::CullModeFlagBits::eBack};

		bool multisample{true};
	};

	class VKPipeline
	{
	public:
		VKPipeline(VKGPUContext *p_ctx, const PipelineCreateInfo &p_create_info);
		~VKPipeline();

		[[nodiscard]] vk::raii::Pipeline &      getPipeline();
		[[nodiscard]] vk::raii::PipelineLayout &getPipelineLayout();

		[[nodiscard]] const PipelineCreateInfo &getCreateInfo() const;

	private:
		void _createGraphicsPipeline();

		static vk::Format _getVulkanAttribType(EShaderDataType p_type);

		VKGPUContext *m_ctx{nullptr};

		PipelineCreateInfo m_createInfo{};

		vk::raii::Pipeline       m_graphicsPipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};
}
