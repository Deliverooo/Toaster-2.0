#pragma once

#include "vk_shader.hpp"
#include "../buffer_layout.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API PipelineSpecInfo
	{
		VertexBufferLayout vertexBufferLayout;
		InstanceLayout     instanceLayout;

		// Thanks to Vulkan 1.3's dynamic rendering, when creating a pipeline, you only need to specify the formats for your rendering attachments
		// This is very helpful because I hate framebuffers and Vulkan render passes. :))))))
		std::vector<vk::Format> colourAttachments;

		vk::Format    depthFormat{vk::Format::eUndefined};
		vk::CompareOp depthCompare{vk::CompareOp::eLess};
		bool          depthTest{true};
		bool          depthWrite{true};

		RefPtr<VKShader> shader{nullptr};

		vk::PrimitiveTopology primitiveTopology{vk::PrimitiveTopology::eTriangleList};
		vk::PolygonMode       polygonMode{vk::PolygonMode::eFill};
		vk::CullModeFlags     cullMode{vk::CullModeFlagBits::eBack};

		bool multisample{false}; // This should not be enabled by default
	};

	class TST_GPU_API VKPipeline
	{
		TST_GPU_OBJECT
	public:
		VKPipeline(VKLogicalDevice *p_device, const PipelineSpecInfo &p_spec_info, const String& p_debug_name = "Pipeline");

		[[nodiscard]] auto getPipeline() -> vk::raii::Pipeline &;
		[[nodiscard]] auto getPipelineLayout() -> const vk::raii::PipelineLayout &;

		[[nodiscard]] auto getSpecInfo() const -> const PipelineSpecInfo &;

		static auto getVulkanAttribType(EBufferDataType p_type) -> vk::Format;

	private:
		auto _createGraphicsPipeline() -> void;

		PipelineSpecInfo m_specInfo{};

		String m_debugName{};

		vk::raii::Pipeline       m_graphicsPipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
	};

	TST_GPU_DEFINE_HANDLE(VKPipeline, Pipeline)
}
