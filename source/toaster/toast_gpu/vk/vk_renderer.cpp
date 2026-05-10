#include "vk_renderer.hpp"

namespace toaster::gpu::render
{
	auto beginRendering(const RenderingInfo &       p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index,
						const RefPtr<VKRenderPass> &p_render_pass) -> void
	{
		TST_ASSERT_MSG(*p_command_buffer, "Command buffer is null");
		TST_ASSERT_MSG(p_render_pass, "Render pass is null");

		std::vector<vk::RenderingAttachmentInfo> colour_rendering_attachment_infos{};
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto &info{colour_rendering_attachment_infos.emplace_back()};

			auto image{rendering_attachment.image};
			if (rendering_attachment.image != nullptr)
			{
				info.imageView = image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
					util::shaderReadToColourAttachment(image.get());

				info.imageLayout = image->getCurrentImageLayout();
			}
			else
			{
				info.imageView   = rendering_attachment.imageView;
				info.imageLayout = rendering_attachment.imageLayout;
			}

			auto resolve_image{rendering_attachment.resolveImage};
			if (resolve_image != nullptr)
			{
				info.resolveImageView = resolve_image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						resolve_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
					util::shaderReadToColourAttachment(resolve_image.get());

				info.resolveImageLayout = resolve_image->getCurrentImageLayout();
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
			auto depth_image{p_rendering_info.pDepthAttachment->image};
			if (depth_image != nullptr)
			{
				depth_attachment_info.imageView = depth_image->getImageView();

				// Perform the layout transition on sampled attachment images
				if ((depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						depth_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					util::shaderReadToDepthAttachment(depth_image.get(), p_rendering_info.depthReadOnly);
				}

				depth_attachment_info.imageLayout = depth_image->getCurrentImageLayout();
			}
			else
			{
				depth_attachment_info.imageView   = p_rendering_info.pDepthAttachment->imageView;
				depth_attachment_info.imageLayout = p_rendering_info.pDepthAttachment->imageLayout;
			}

			auto depth_resolve_image{p_rendering_info.pDepthAttachment->resolveImage};
			if (depth_resolve_image != nullptr)
			{
				depth_attachment_info.resolveImageView = depth_resolve_image->getImageView();
				// Perform the layout transition on sampled attachment images
				if ((depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled) && (
						depth_resolve_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal))
				{
					util::shaderReadToDepthAttachment(depth_resolve_image.get(), false);
				}

				depth_attachment_info.resolveImageLayout = depth_resolve_image->getCurrentImageLayout();
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
		rendering_info.colorAttachmentCount = colour_rendering_attachment_infos.empty() ? 0u : p_rendering_info.colourAttachments.size();
		rendering_info.pColorAttachments    = colour_rendering_attachment_infos.empty() ? nullptr : colour_rendering_attachment_infos.data();
		rendering_info.pDepthAttachment     = p_rendering_info.pDepthAttachment ? &depth_attachment_info : nullptr;
		rendering_info.pStencilAttachment   = p_rendering_info.pStencilAttachment ? &stencil_attachment_info : nullptr;

		const vk::Extent2D rendering_extent{p_rendering_info.renderArea.extent};
		const vk::Offset2D rendering_offset{p_rendering_info.renderArea.offset};

		const vk::Viewport viewport{
			static_cast<float32>(rendering_offset.x),
			static_cast<float32>(rendering_offset.y),
			static_cast<float32>(rendering_extent.width),
			static_cast<float32>(rendering_extent.height),
			0.0f,
			1.0f
		};
		const vk::Rect2D scissor{rendering_offset, rendering_extent};

		p_command_buffer.beginRendering(rendering_info);
		p_command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipeline());
		p_command_buffer.setViewport(0, viewport);
		p_command_buffer.setScissor(0, scissor);

		p_render_pass->update(p_frame_index);

		const auto descriptor_sets = p_render_pass->getDescriptorSets(p_frame_index);
		if (!descriptor_sets.empty())
			p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(), p_render_pass->getStartSetIndex(),
												descriptor_sets, nullptr);
	}

	auto endRendering(const RenderingInfo &p_rendering_info, const vk::raii::CommandBuffer &p_command_buffer) -> void
	{
		TST_ASSERT_MSG(*p_command_buffer, "Command buffer is null");

		p_command_buffer.endRendering();

		// Perform the layout transition on sampled attachment images
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto image{rendering_attachment.image};
			if ((image != nullptr) && (image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				util::colourAttachmentToShaderRead(image.get());
			}
			auto resolve_image{rendering_attachment.resolveImage};
			if ((resolve_image != nullptr) && (resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				util::colourAttachmentToShaderRead(resolve_image.get());
			}
		}

		if (p_rendering_info.pDepthAttachment != nullptr)
		{
			auto depth_image{p_rendering_info.pDepthAttachment->image};

			if ((depth_image != nullptr) && (depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				util::depthAttachmentToShaderRead(depth_image.get(), p_rendering_info.depthReadOnly);
			}
			auto depth_resolve_image{p_rendering_info.pDepthAttachment->resolveImage};
			if ((depth_resolve_image != nullptr) && (depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				util::depthAttachmentToShaderRead(depth_resolve_image.get(), false);
			}
		}
	}

	auto beginCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<VKComputePass> &p_compute_pass) -> void
	{
		p_command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipeline());

		p_compute_pass->update(p_frame_index);

		const auto descriptor_sets = p_compute_pass->getDescriptorSets(p_frame_index);
		if (!descriptor_sets.empty())
			p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipelineLayout(), p_compute_pass->getStartSetIndex(),
												descriptor_sets, nullptr);
	}

	auto dispatchCompute(const vk::raii::CommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<VKComputePass> &p_compute_pass,
						 const RefPtr<VKMaterial> &     p_material, uint32       p_work_group_x, uint32 p_work_group_y, uint32 p_work_group_z) -> void
	{
		if (p_material)
			if (p_material->hasDescriptorSets())
				if (const auto descriptor_set{p_material->getDescriptorSet(p_frame_index)})
					p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipelineLayout(), 0, descriptor_set, nullptr);

		p_command_buffer.dispatch(p_work_group_x, p_work_group_y, p_work_group_z);
	}

	auto endCompute([[maybe_unused]] const vk::raii::CommandBuffer &p_command_buffer, [[maybe_unused]] uint32 p_frame_index,
					[[maybe_unused]] const RefPtr<VKComputePass> &  p_compute_pass) -> void
	{
	}

	auto renderGeometry(const vk::raii::CommandBuffer &p_command_buffer, uint32                      p_frame_index, const RefPtr<VKPipeline> &p_pipeline,
						const RefPtr<VKVertexBuffer> & p_vertex_buffer, const RefPtr<VKIndexBuffer> &p_index_buffer, uint32                   p_index_count,
						const RefPtr<VKMaterial> &     p_material, const glm::mat4 &                 p_transform) -> void
	{
		// Push the constants
		p_command_buffer.pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		if (p_material)
		{
			if (p_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(p_frame_index)};
				p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});
			}

			const auto &push_constants{p_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = sizeof(glm::mat4);
				push_constants_info.pValues    = push_constants.data();

				p_command_buffer.pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_vertex_buffer->bind(p_command_buffer);
		p_index_buffer->bind(p_command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(p_index_count, 1, 0, 0, 0);
	}

	auto renderMesh(const vk::raii::CommandBuffer &p_command_buffer, uint32     p_frame_index, const RefPtr<VKMesh> &p_mesh, uint32 p_submesh_index,
					const RefPtr<VKPipeline> &     p_pipeline, const glm::mat4 &p_transform) -> void
	{
		TST_ASSERT_MSG(*p_command_buffer, "Command buffer is null");

		// Push the constants
		p_command_buffer.pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		const auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};
		auto        material{p_mesh->getMaterials()[submesh.materialIndex]};

		if (material) // You technically don't need to use a material if you don't want to
		{
			if (material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{material->getDescriptorSet(p_frame_index)};
				p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});
			}

			const auto &push_constants{material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = sizeof(glm::mat4);
				push_constants_info.pValues    = push_constants.data();

				p_command_buffer.pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_mesh->getVertexBuffer()->bind(p_command_buffer);
		p_mesh->getIndexBuffer()->bind(p_command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(submesh.indexCount, 1, submesh.baseIndex, submesh.baseVertex, 0);
	}

	auto renderMesh(const vk::raii::CommandBuffer &p_command_buffer, uint32     p_frame_index, const RefPtr<VKMesh> &  p_mesh, uint32 p_submesh_index,
					const RefPtr<VKPipeline> &     p_pipeline, const glm::mat4 &p_transform, const RefPtr<VKMaterial> &p_override_material) -> void
	{
		TST_ASSERT_MSG(*p_command_buffer, "Command buffer is null");

		// Push the constants
		p_command_buffer.pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		const auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};

		if (p_override_material) // You technically don't need to use a material if you don't want to
		{
			if (p_override_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_override_material->getDescriptorSet(p_frame_index)};
				p_command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0, material_descriptor_set, {});
			}

			const auto &push_constants{p_override_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = sizeof(glm::mat4);
				push_constants_info.pValues    = push_constants.data();

				p_command_buffer.pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_mesh->getVertexBuffer()->bind(p_command_buffer);
		p_mesh->getIndexBuffer()->bind(p_command_buffer, vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer.drawIndexed(submesh.indexCount, 1, submesh.baseIndex, static_cast<int32>(submesh.baseVertex), 0);
	}
}
