#include "renderer.hpp"

#include "globals.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster
{
	void Renderer::beginRendering(const gpu::RenderingInfo &       p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
								  const RefPtr<gpu::VKRenderPass> &p_render_pass)
	{
		const vk::Extent2D rendering_extent{p_rendering_info.renderArea.extent};

		const vk::Viewport viewport{0.0f, 0.0f, static_cast<float32>(rendering_extent.width), static_cast<float32>(rendering_extent.height), 0.0f, 1.0f};
		const vk::Rect2D   scissor{vk::Offset2D{0, 0}, rendering_extent};

		std::vector<vk::RenderingAttachmentInfo> colour_rendering_attachment_infos{};
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto &info{colour_rendering_attachment_infos.emplace_back()};

			if (rendering_attachment.image != nullptr)
			{
				info.imageView   = rendering_attachment.image->getImageView();
				info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;

				// Perform the layout transition on sampled attachment images
				if ((rendering_attachment.image->getCreateInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						rendering_attachment.image->getCurrentImageLayout() != vk::ImageLayout::eColorAttachmentOptimal))
				{
					rendering_attachment.image->getContext()->transitionImageLayout(rendering_attachment.image->getImage(), vk::ImageLayout::eShaderReadOnlyOptimal,
																					vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eShaderRead,
																					vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eFragmentShader,
																					vk::PipelineStageFlagBits::eColorAttachmentOutput,
																					rendering_attachment.image->getCreateInfo().mipCount,
																					vk::ImageAspectFlagBits::eColor);
					rendering_attachment.image->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
				}
			}
			else
			{
				info.imageView   = rendering_attachment.imageView;
				info.imageLayout = rendering_attachment.imageLayout;
			}

			if (rendering_attachment.resolveImage != nullptr)
			{
				info.resolveImageView   = rendering_attachment.resolveImage->getImageView();
				info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;

				// Perform the layout transition on sampled attachment images
				if ((rendering_attachment.resolveImage->getCreateInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						rendering_attachment.resolveImage->getCurrentImageLayout() != vk::ImageLayout::eColorAttachmentOptimal))
				{
					rendering_attachment.resolveImage->getContext()->transitionImageLayout(rendering_attachment.resolveImage->getImage(),
																						   vk::ImageLayout::eShaderReadOnlyOptimal,
																						   vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eShaderRead,
																						   vk::AccessFlagBits::eColorAttachmentWrite,
																						   vk::PipelineStageFlagBits::eFragmentShader,
																						   vk::PipelineStageFlagBits::eColorAttachmentOutput,
																						   rendering_attachment.resolveImage->getCreateInfo().mipCount,
																						   vk::ImageAspectFlagBits::eColor);
					rendering_attachment.resolveImage->setCurrentImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
				}
			}
			else
			{
				info.resolveImageView   = rendering_attachment.resolveImageView;
				info.resolveImageLayout = rendering_attachment.resolveImageLayout;
			}

			info.resolveMode = rendering_attachment.resolveMode;

			info.loadOp     = rendering_attachment.loadOp;
			info.storeOp    = rendering_attachment.storeOp;
			info.clearValue = rendering_attachment.clearValue;
		}

		vk::RenderingAttachmentInfo depth_attachment_info{};
		if (p_rendering_info.pDepthAttachment != nullptr)
		{
			if (p_rendering_info.pDepthAttachment->image != nullptr)
			{
				depth_attachment_info.imageView   = p_rendering_info.pDepthAttachment->image->getImageView();
				depth_attachment_info.imageLayout = p_rendering_info.pDepthAttachment->image->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.imageView   = p_rendering_info.pDepthAttachment->imageView;
				depth_attachment_info.imageLayout = p_rendering_info.pDepthAttachment->imageLayout;
			}

			if (p_rendering_info.pDepthAttachment->resolveImage != nullptr)
			{
				depth_attachment_info.resolveImageView   = p_rendering_info.pDepthAttachment->resolveImage->getImageView();
				depth_attachment_info.resolveImageLayout = p_rendering_info.pDepthAttachment->resolveImage->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.resolveImageView   = p_rendering_info.pDepthAttachment->resolveImageView;
				depth_attachment_info.resolveImageLayout = p_rendering_info.pDepthAttachment->resolveImageLayout;
			}

			depth_attachment_info.resolveMode = p_rendering_info.pDepthAttachment->resolveMode;

			depth_attachment_info.loadOp     = p_rendering_info.pDepthAttachment->loadOp;
			depth_attachment_info.storeOp    = p_rendering_info.pDepthAttachment->storeOp;
			depth_attachment_info.clearValue = p_rendering_info.pDepthAttachment->clearValue;
		}

		vk::RenderingAttachmentInfo stencil_attachment_info{};
		if (p_rendering_info.pStencilAttachment != nullptr)
		{
			if (p_rendering_info.pStencilAttachment->image != nullptr)
			{
				stencil_attachment_info.imageView   = p_rendering_info.pStencilAttachment->image->getImageView();
				stencil_attachment_info.imageLayout = p_rendering_info.pStencilAttachment->image->getCurrentImageLayout();
			}
			else
			{
				stencil_attachment_info.imageView   = p_rendering_info.pStencilAttachment->imageView;
				stencil_attachment_info.imageLayout = p_rendering_info.pStencilAttachment->imageLayout;
			}

			if (p_rendering_info.pStencilAttachment->resolveImage != nullptr)
			{
				stencil_attachment_info.resolveImageView   = p_rendering_info.pStencilAttachment->resolveImage->getImageView();
				stencil_attachment_info.resolveImageLayout = p_rendering_info.pStencilAttachment->resolveImage->getCurrentImageLayout();
			}
			else
			{
				stencil_attachment_info.resolveImageView   = p_rendering_info.pStencilAttachment->resolveImageView;
				stencil_attachment_info.resolveImageLayout = p_rendering_info.pStencilAttachment->resolveImageLayout;
			}

			stencil_attachment_info.resolveMode = p_rendering_info.pStencilAttachment->resolveMode;

			stencil_attachment_info.loadOp     = p_rendering_info.pStencilAttachment->loadOp;
			stencil_attachment_info.storeOp    = p_rendering_info.pStencilAttachment->storeOp;
			stencil_attachment_info.clearValue = p_rendering_info.pStencilAttachment->clearValue;
		}

		vk::RenderingInfo rendering_info{};
		rendering_info.flags                = p_rendering_info.flags;
		rendering_info.renderArea           = p_rendering_info.renderArea;
		rendering_info.layerCount           = p_rendering_info.layerCount;
		rendering_info.colorAttachmentCount = p_rendering_info.colourAttachments.size();
		rendering_info.pColorAttachments    = colour_rendering_attachment_infos.data();
		rendering_info.pDepthAttachment     = p_rendering_info.pDepthAttachment ? &depth_attachment_info : nullptr;
		rendering_info.pStencilAttachment   = p_rendering_info.pStencilAttachment ? &stencil_attachment_info : nullptr;

		p_command_buffer.beginRendering(rendering_info);
		p_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipeline());
		p_command_buffer.setViewport(0, viewport);
		p_command_buffer.setScissor(0, scissor);

		p_render_pass->update(p_frame_index);

		const auto descriptor_sets = p_render_pass->getDescriptorSets(p_frame_index);
		p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(), p_render_pass->getStartSetIndex(),
											descriptor_sets, nullptr);
	}

	void Renderer::endRendering(const gpu::RenderingInfo &p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer)
	{
		p_command_buffer.endRendering();

		// Perform the layout transition on sampled attachment images
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			if ((rendering_attachment.image != nullptr) && (rendering_attachment.image->getCreateInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				rendering_attachment.image->getContext()->transitionImageLayout(rendering_attachment.image->getImage(), vk::ImageLayout::eColorAttachmentOptimal,
																				vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eColorAttachmentWrite,
																				vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eColorAttachmentOutput,
																				vk::PipelineStageFlagBits::eFragmentShader,
																				rendering_attachment.image->getCreateInfo().mipCount, vk::ImageAspectFlagBits::eColor);
				rendering_attachment.image->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			}
			if ((rendering_attachment.resolveImage != nullptr) && (rendering_attachment.resolveImage->getCreateInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				rendering_attachment.resolveImage->getContext()->transitionImageLayout(rendering_attachment.resolveImage->getImage(),
																					   vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
																					   vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead,
																					   vk::PipelineStageFlagBits::eColorAttachmentOutput,
																					   vk::PipelineStageFlagBits::eFragmentShader,
																					   rendering_attachment.resolveImage->getCreateInfo().mipCount,
																					   vk::ImageAspectFlagBits::eColor);
				rendering_attachment.resolveImage->setCurrentImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
			}
		}
	}

	void Renderer::renderGeometry(const vk::raii::CommandBuffer &    p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
								  const RefPtr<gpu::VKVertexBuffer> &p_vertex_buffer, const RefPtr<gpu::VKIndexBuffer> &p_index_buffer, uint32 p_index_count,
								  const RefPtr<gpu::VKMaterial> &    p_material, const glm::mat4 &p_transform)
	{
		// Push the constants
		p_command_buffer.pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		if (p_material->hasDescriptorSets())
		{
			// Bind the material descriptor set (0)
			vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(p_frame_index)};
			p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});
		}
		// Bind the vertex and index buffers
		p_vertex_buffer->bind(p_command_buffer);
		p_index_buffer->bind(p_command_buffer, vk::IndexType::eUint16);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(p_index_count, 1, 0, 0, 0);
	}

	void Renderer::renderFullscreenQuad(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
										const RefPtr<gpu::VKMaterial> &p_material)
	{
		// Push the constants
		p_command_buffer.pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, glm::mat4{1.0f});

		if (p_material->hasDescriptorSets())
		{
			// Bind the material descriptor set (0)
			vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(p_frame_index)};
			p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});
		}
		// Bind the vertex and index buffers
		Globals::getFullscreenQuadVertexBuffer()->bind(p_command_buffer);
		Globals::getFullscreenQuadIndexBuffer()->bind(p_command_buffer, vk::IndexType::eUint16);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(Globals::getFullscreenQuadIndices().size(), 1, 0, 0, 0);
	}
}
