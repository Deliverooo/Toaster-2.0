#pragma once

#include "toast_render.hpp"

#include "toast_gpu/buffer_layout.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"

#include "toast_gpu/vk/vk_shader.hpp"

namespace toaster::render
{
	class RenderContext;

	class TST_RENDER_API GraphicsState
	{
		TST_RENDER_OBJECT
	public:
		struct ColourBlendAttachmentInfo
		{
			bool32                    blendEnable{false};
			vk::ColorBlendEquationEXT blendEquation{};
			vk::ColorComponentFlags   colourWriteMask{
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
			};
		};

		GraphicsState(RenderContext &p_render_ctx);

		auto setShaders(const std::vector<gpu::ShaderHandle> &p_shaders) -> GraphicsState &
		{
			m_shaders = p_shaders;
			return *this;
		}

		auto setVertexBufferLayout(const gpu::VertexBufferLayout &p_layout) -> GraphicsState &
		{
			m_vertexBufferLayout = p_layout;
			return *this;
		}

		auto setAttachmentCount(uint32 p_attachment_count) -> GraphicsState &
		{
			m_attachmentCount = p_attachment_count;
			m_colourBlendAttachmentInfos.resize(m_attachmentCount);
			return *this;
		}

		auto setCullMode(vk::CullModeFlagBits p_cull_mode) -> GraphicsState &
		{
			m_cullMode = p_cull_mode;
			return *this;
		}

		auto setEnableMultisample(bool32 p_enable) -> GraphicsState &
		{
			m_enableMultisample = p_enable;
			return *this;
		}

		auto setEnableDepthTest(bool32 p_enable) -> GraphicsState &
		{
			m_depthTestEnable = p_enable;
			return *this;
		}

		auto setEnableDepthWrite(bool32 p_enable) -> GraphicsState &
		{
			m_depthWriteEnable = p_enable;
			return *this;
		}

		auto setEnableDepthCompareOp(vk::CompareOp p_compare_op) -> GraphicsState &
		{
			m_depthCompareOp = p_compare_op;
			return *this;
		}

		auto setColourBlendAttachmentInfo(uint32 p_attachment_index, const ColourBlendAttachmentInfo &p_info) -> GraphicsState &
		{
			m_colourBlendAttachmentInfos.at(p_attachment_index) = p_info;
			return *this;
		}

		auto bind(gpu::VKCommandBuffer *p_command_buffer = nullptr) const -> void;

	private:
		std::vector<gpu::ShaderHandle> m_shaders;

		gpu::VertexBufferLayout m_vertexBufferLayout;

		uint32                                 m_attachmentCount{0u};
		std::vector<ColourBlendAttachmentInfo> m_colourBlendAttachmentInfos;

		vk::CullModeFlagBits m_cullMode{vk::CullModeFlagBits::eBack};
		volatile bool32      m_enableMultisample{false};

		bool32 m_depthTestEnable{false};
		bool32 m_depthWriteEnable{false};

		vk::CompareOp m_depthCompareOp{vk::CompareOp::eLess};
	};

	TST_RENDER_DEFINE_HANDLE(GraphicsState, GraphicsState);
}
