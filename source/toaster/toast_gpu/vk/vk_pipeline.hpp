#pragma once

#include "vk_gpu_context.hpp"

#include "../vertex_buffer_layout.hpp"

namespace toaster::gpu
{
	struct PipelineCreateInfo
	{
		VertexBufferLayout vertexBufferLayout;

		vk::Format colourAttachmentFormat;
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

		PipelineCreateInfo m_createInfo;

		vk::raii::Pipeline       m_graphicsPipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};
}
