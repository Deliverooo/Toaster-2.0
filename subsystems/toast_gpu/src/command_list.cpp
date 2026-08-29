#include "toast_gpu/command_list.hpp"

#include "toast_gpu/command_pool.hpp"

namespace toaster::gpu
{
	auto getLoadOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentLoadOp
	{
		switch (p_usage_op)
		{
			case EAttachmentUsageOP::eClearStore:
			case EAttachmentUsageOP::eClearNone:
			case EAttachmentUsageOP::eClearDontCare:
				return vk::AttachmentLoadOp::eClear;
			case EAttachmentUsageOP::eLoadStore:
			case EAttachmentUsageOP::eLoadNone:
			case EAttachmentUsageOP::eLoadDontCare:
				return vk::AttachmentLoadOp::eLoad;
			case EAttachmentUsageOP::eNoneStore:
			case EAttachmentUsageOP::eNoneNone:
			case EAttachmentUsageOP::eNoneDontCare:
				return vk::AttachmentLoadOp::eNone;
			case EAttachmentUsageOP::eDontCareStore:
			case EAttachmentUsageOP::eDontCareNone:
			case EAttachmentUsageOP::eDontCareDontCare:
				return vk::AttachmentLoadOp::eDontCare;
		}
		return vk::AttachmentLoadOp::eNone;
	}

	auto getStoreOp(EAttachmentUsageOP p_usage_op) -> vk::AttachmentStoreOp
	{
		switch (p_usage_op)
		{
			case EAttachmentUsageOP::eClearStore:
			case EAttachmentUsageOP::eLoadStore:
			case EAttachmentUsageOP::eNoneStore:
			case EAttachmentUsageOP::eDontCareStore:
				return vk::AttachmentStoreOp::eStore;
			case EAttachmentUsageOP::eClearNone:
			case EAttachmentUsageOP::eLoadNone:
			case EAttachmentUsageOP::eNoneNone:
			case EAttachmentUsageOP::eDontCareNone:
				return vk::AttachmentStoreOp::eNone;
			case EAttachmentUsageOP::eClearDontCare:
			case EAttachmentUsageOP::eLoadDontCare:
			case EAttachmentUsageOP::eNoneDontCare:
			case EAttachmentUsageOP::eDontCareDontCare:
				return vk::AttachmentStoreOp::eDontCare;
		}
		return vk::AttachmentStoreOp::eNone;
	}

	auto getResolveMode(EAttachmentResolveMode p_resolve_mode) -> vk::ResolveModeFlagBits
	{
		switch (p_resolve_mode)
		{
			case EAttachmentResolveMode::eNone: return vk::ResolveModeFlagBits::eNone;
			case EAttachmentResolveMode::eZero: return vk::ResolveModeFlagBits::eSampleZero;
			case EAttachmentResolveMode::eAverage: return vk::ResolveModeFlagBits::eAverage;
			case EAttachmentResolveMode::eMin: return vk::ResolveModeFlagBits::eMin;
			case EAttachmentResolveMode::eMax: return vk::ResolveModeFlagBits::eMax;
		}
		return vk::ResolveModeFlagBits::eNone;
	}

	CommandList::CommandList(CommandPool& p_command_pool, Device& p_device, vk::CommandBuffer p_cmd) :
	m_commandPool(&p_command_pool),	m_device(&p_device), m_cmd(p_cmd)
	{
	}

	CommandList::CommandList(CommandList &&p_other) noexcept : m_commandPool(p_other.m_commandPool),
	m_device(p_other.m_device),
	m_cmd(p_other.m_cmd)
	{
		p_other.m_commandPool = nullptr;
		p_other.m_device = nullptr;
		p_other.m_cmd = nullptr;
	}

	CommandList &CommandList::operator=(CommandList &&p_other) noexcept
	{
		if (this != &p_other)
		{
			m_commandPool = p_other.m_commandPool;
			m_device = p_other.m_device;
			m_cmd = p_other.m_cmd;

			p_other.m_commandPool = nullptr;
			p_other.m_device = nullptr;
			p_other.m_cmd = nullptr;
		}
		return *this;
	}

	auto CommandList::reset() -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		if (!(m_commandPool->m_commandPoolFlags & ECommandPoolBits::eReset))
		{
			TST_ASSERT_MSG(false,"Command pool was not created with the reset bit");
		}
		m_cmd.reset();
	}

	auto CommandList::begin() -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		m_cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
	}

	auto CommandList::end() -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		m_cmd.end();
	}

	auto CommandList::beginRendering(const RenderingInfo &p_rendering_info) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		std::vector<vk::RenderingAttachmentInfo> colour_attachments(p_rendering_info.colourAttachments.size());
		for (uint32 i{0u}; i < p_rendering_info.colourAttachments.size(); ++i)
		{
			auto &src_attachment{p_rendering_info.colourAttachments[i]};
			auto &dst_attachment{colour_attachments[i]};

			const TextureData *render_target_data{m_device->getTextureData(src_attachment.renderTarget)};
			TST_ASSERT(render_target_data);
			dst_attachment.imageView   = render_target_data->imageView;
			dst_attachment.imageLayout = render_target_data->layout;

			dst_attachment.loadOp     = getLoadOp(src_attachment.usageOp);
			dst_attachment.storeOp    = getStoreOp(src_attachment.usageOp);
			dst_attachment.clearValue = *reinterpret_cast<const vk::ClearValue *>(&src_attachment.clearValue); // They should have the same memory layout

			const TextureData *resolve_render_target_data{m_device->getTextureData(src_attachment.resolveTarget)};
			if (resolve_render_target_data)
			{
				dst_attachment.resolveImageView   = resolve_render_target_data->imageView;
				dst_attachment.resolveImageLayout = resolve_render_target_data->layout;
				dst_attachment.resolveMode        = getResolveMode(src_attachment.resolveMode);
			}
			else
			{
				dst_attachment.resolveMode        = vk::ResolveModeFlagBits::eNone;
				dst_attachment.resolveImageView   = nullptr;
				dst_attachment.resolveImageLayout = vk::ImageLayout::eUndefined;
			}
		}

		vk::RenderingAttachmentInfo depth_attachment{};
		if (p_rendering_info.depthAttachment.has_value())
		{
			auto &             src_attachment{p_rendering_info.depthAttachment.value()};
			const TextureData *render_target_data{m_device->getTextureData(src_attachment.renderTarget)};
			TST_ASSERT(render_target_data);

			depth_attachment.imageView   = render_target_data->imageView;
			depth_attachment.imageLayout = render_target_data->layout;

			depth_attachment.loadOp     = getLoadOp(src_attachment.usageOp);
			depth_attachment.storeOp    = getStoreOp(src_attachment.usageOp);
			depth_attachment.clearValue = *reinterpret_cast<const vk::ClearValue *>(&src_attachment.clearValue); // They should have the same memory layout

			const TextureData *resolve_render_target_data{m_device->getTextureData(src_attachment.resolveTarget)};
			if (resolve_render_target_data)
			{
				depth_attachment.resolveImageView   = resolve_render_target_data->imageView;
				depth_attachment.resolveImageLayout = resolve_render_target_data->layout;
				depth_attachment.resolveMode        = getResolveMode(src_attachment.resolveMode);
			}
			else
			{
				depth_attachment.resolveMode        = vk::ResolveModeFlagBits::eNone;
				depth_attachment.resolveImageView   = nullptr;
				depth_attachment.resolveImageLayout = vk::ImageLayout::eUndefined;
			}
		}

		vk::RenderingAttachmentInfo stencil_attachment{};
		if (p_rendering_info.stencilAttachment.has_value())
		{
			auto &             src_attachment{p_rendering_info.stencilAttachment.value()};
			const TextureData *render_target_data{m_device->getTextureData(src_attachment.renderTarget)};
			TST_ASSERT(render_target_data);

			stencil_attachment.imageView   = render_target_data->imageView;
			stencil_attachment.imageLayout = render_target_data->layout;

			stencil_attachment.loadOp     = getLoadOp(src_attachment.usageOp);
			stencil_attachment.storeOp    = getStoreOp(src_attachment.usageOp);
			stencil_attachment.clearValue = *reinterpret_cast<const vk::ClearValue *>(&src_attachment.clearValue); // They should have the same memory layout

			const TextureData *resolve_render_target_data{m_device->getTextureData(src_attachment.resolveTarget)};
			if (resolve_render_target_data)
			{
				stencil_attachment.resolveImageView   = resolve_render_target_data->imageView;
				stencil_attachment.resolveImageLayout = resolve_render_target_data->layout;
				stencil_attachment.resolveMode        = getResolveMode(src_attachment.resolveMode);
			}
			else
			{
				stencil_attachment.resolveMode        = vk::ResolveModeFlagBits::eNone;
				stencil_attachment.resolveImageView   = nullptr;
				stencil_attachment.resolveImageLayout = vk::ImageLayout::eUndefined;
			}
		}

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{
			vk::Offset2D{p_rendering_info.renderArea.offset.x, p_rendering_info.renderArea.offset.y},
			vk::Extent2D{p_rendering_info.renderArea.size.x, p_rendering_info.renderArea.size.y}
		};
		rendering_info.layerCount           = 1u;
		rendering_info.viewMask             = 0u;
		rendering_info.colorAttachmentCount = colour_attachments.size();
		rendering_info.pColorAttachments    = colour_attachments.data();
		rendering_info.pDepthAttachment     = p_rendering_info.depthAttachment.has_value() ? &depth_attachment : nullptr;
		rendering_info.pStencilAttachment   = p_rendering_info.stencilAttachment.has_value() ? &stencil_attachment : nullptr;

		m_cmd.beginRendering(rendering_info, FunctionDispatcher::get());
	}

	auto CommandList::endRendering() -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		m_cmd.endRendering();
	}

	auto CommandList::setViewport(const tsm::Viewport &p_viewport) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		const vk::Viewport vk_viewport{
			p_viewport.offset.x,
			p_viewport.offset.y,
			p_viewport.size.x,
			p_viewport.size.y,
			p_viewport.depthBounds.x,
			p_viewport.depthBounds.y
		};
		m_cmd.setViewportWithCountEXT(vk_viewport, FunctionDispatcher::get());
	}

	auto CommandList::setScissor(const tsm::Rect &p_scissor) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		const vk::Rect2D vk_scissor{{p_scissor.offset.x, p_scissor.offset.y}, {p_scissor.size.x, p_scissor.size.y}};
		m_cmd.setScissorWithCountEXT(vk_scissor, FunctionDispatcher::get());
	}

	auto CommandList::bindResourceHeap(const ResourceDescriptorHeap &p_resource_heap) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		m_cmd.bindResourceHeapEXT(p_resource_heap.getBindInfo(), FunctionDispatcher::get());
	}

	auto CommandList::bindSamplerHeap(const SamplerDescriptorHeap &p_sampler_heap) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		m_cmd.bindSamplerHeapEXT(p_sampler_heap.getBindInfo(), FunctionDispatcher::get());
	}

	auto CommandList::bindShaders(const InitialiserList<const ShaderHandle> &p_shaders) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		std::vector<vk::ShaderStageFlagBits> stages(p_shaders.size());
		std::vector<vk::ShaderEXT>           shaders(p_shaders.size());

		uint32 i{0u};
		for (const auto &shader: p_shaders)
		{
			const ShaderData *shader_data{m_device->getShaderData(shader)};
			TST_ASSERT(shader_data);

			const auto stage{static_cast<vk::ShaderStageFlagBits>(static_cast<vk::ShaderStageFlags::MaskType>(Device::getVulkanShaderStages(shader_data->stage)))};
			stages[i]  = stage;
			shaders[i] = shader_data->shader;
			++i;
		}

		m_cmd.bindShadersEXT(stages, shaders, FunctionDispatcher::get());
	}

	auto CommandList::copyBuffer(BufferHandle p_src_buffer, BufferHandle p_dst_buffer, uint64 p_size, uint64 p_src_offset, uint64 p_dst_offset) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		const BufferData *src_data{m_device->getBufferData(p_src_buffer)};
		BufferData *      dst_data{m_device->getBufferData(p_dst_buffer)};

		TST_ASSERT(src_data && dst_data);
		TST_ASSERT_MSG(src_data->usageFlags & vk::BufferUsageFlagBits::eTransferSrc, "Src buffer is not a transfer src");
		TST_ASSERT_MSG(dst_data->usageFlags & vk::BufferUsageFlagBits::eTransferDst, "Dst buffer is not a transfer dst");

		vk::BufferCopy2 copy_region{};
		copy_region.srcOffset = p_src_offset;
		copy_region.dstOffset = p_dst_offset;
		copy_region.size      = p_size;

		vk::CopyBufferInfo2 copy_buffer_info{};
		copy_buffer_info.srcBuffer = src_data->buffer;
		copy_buffer_info.dstBuffer = dst_data->buffer;
		copy_buffer_info.setRegions(copy_region);
		m_cmd.copyBuffer2(copy_buffer_info);
	}

	auto CommandList::copyBufferToTexture(BufferHandle p_src_buffer, TextureHandle p_dst_texture) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		const BufferData *src_data{m_device->getBufferData(p_src_buffer)};
		TextureData *     dst_data{m_device->getTextureData(p_dst_texture)};
		TST_ASSERT(src_data && dst_data);
		TST_ASSERT_MSG(src_data->usageFlags & vk::BufferUsageFlagBits::eTransferSrc, "Src buffer is not a transfer src");
		TST_ASSERT_MSG(dst_data->layout == vk::ImageLayout::eTransferDstOptimal, "Dst texture is not in the transfer dst layout");

		vk::BufferImageCopy2 buffer_image_copy{};
		buffer_image_copy.bufferOffset      = 0;
		buffer_image_copy.bufferRowLength   = 0;
		buffer_image_copy.bufferImageHeight = 0;
		buffer_image_copy.imageOffset       = vk::Offset3D{0, 0, 0};
		buffer_image_copy.imageExtent       = dst_data->extent;
		buffer_image_copy.imageSubresource  = {getImageAspectMask(dst_data->format), 0, 0, dst_data->layerCount};

		vk::CopyBufferToImageInfo2 copy_buffer_to_image_info{};
		copy_buffer_to_image_info.srcBuffer      = src_data->buffer;
		copy_buffer_to_image_info.dstImage       = dst_data->image;
		copy_buffer_to_image_info.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
		copy_buffer_to_image_info.setRegions(buffer_image_copy);

		m_cmd.copyBufferToImage2(copy_buffer_to_image_info);
	}

	auto CommandList::transitionTextureLayout(TextureHandle p_texture, vk::ImageLayout p_dst_layout) -> void
	{
		[[unlikely]]TST_ASSERT(m_cmd);
		TextureData *texture_data{m_device->getTextureData(p_texture)};
		TST_ASSERT(texture_data);

		TST_ASSERT(p_dst_layout != vk::ImageLayout::eUndefined);
		if (texture_data->layout == p_dst_layout) // If the image is already in the specified layout, there is no need to transition.
			return;

		vk::ImageMemoryBarrier2 image_memory_barrier{};
		image_memory_barrier.image     = texture_data->image;
		image_memory_barrier.oldLayout = texture_data->layout;
		image_memory_barrier.newLayout = p_dst_layout;

		getImageAccessFlagsAndStageMask(texture_data->layout, image_memory_barrier.srcAccessMask, image_memory_barrier.srcStageMask);
		getImageAccessFlagsAndStageMask(p_dst_layout, image_memory_barrier.dstAccessMask, image_memory_barrier.dstStageMask);

		image_memory_barrier.subresourceRange = vk::ImageSubresourceRange{
			getImageAspectMask(texture_data->format),
			0u,
			texture_data->mipLevels,
			0u,
			texture_data->layerCount
		};

		vk::DependencyInfo dependency_info{};
		dependency_info.setImageMemoryBarriers(image_memory_barrier);

		m_cmd.pipelineBarrier2(dependency_info);

		texture_data->layout = p_dst_layout; // Maybe I shouldn't update the layout immediately, but it doesn't matter
	}
}
