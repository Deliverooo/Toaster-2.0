#include "render_context.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"

#include "globals.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"

#include "material.hpp"
#include "mesh.hpp"

namespace toaster::render
{
	RenderContext::RenderContext(const RenderContextSpecInfo &p_spec_info) : m_specInfo(p_spec_info)
	{
		bool use_present{m_specInfo.instanceExtensions.contains(VK_KHR_SURFACE_EXTENSION_NAME)};
		#pragma region create vulkan objects
		gpu::VKInstanceSpecInfo vk_instance_spec_info{};
		vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan";
		vk_instance_spec_info.requiredExtensions = m_specInfo.instanceExtensions;
		vk_instance_spec_info.printDebugInfo     = m_specInfo.printDebugInfo;
		m_backendInstance                        = new gpu::VKInstance{vk_instance_spec_info};

		std::unordered_set<String> required_device_extensions{
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRMaintenance6ExtensionName,
			vk::KHRLoadStoreOpNoneExtensionName,
			vk::KHRShaderNonSemanticInfoExtensionName
		};
		if (use_present)
			required_device_extensions.insert(vk::KHRSwapchainExtensionName);

		gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
		vk_physical_device_spec_info.requiredExtensions = required_device_extensions;
		vk_physical_device_spec_info.printDebugInfo     = m_specInfo.printDebugInfo;

		m_physicalDevice = new gpu::VKPhysicalDevice{m_backendInstance, vk_physical_device_spec_info};

		gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
		vk_logical_device_spec_info.usePresent           = use_present;
		vk_logical_device_spec_info.requiredExtensions   = required_device_extensions;
		vk_logical_device_spec_info.printDebugInfo       = m_specInfo.printDebugInfo;
		vk_logical_device_spec_info.printShaderDebugInfo = m_specInfo.printDebugInfo;
		auto features{gpu::VKLogicalDeviceSpecInfo::getDefaultFeatures()};
		vk_logical_device_spec_info.pNext = features.get<vk::PhysicalDeviceFeatures2>();

		m_logicalDevice = new gpu::VKLogicalDevice{m_physicalDevice, vk_logical_device_spec_info};
		#pragma endregion

		if (m_specInfo.createGlobals)
			m_globals = new Globals{m_logicalDevice, m_specInfo.binaryDir};
	}

	RenderContext::~RenderContext()
	{
		delete m_globals;

		// Clean up any remaining objects
		performGarbageCollection();

		delete m_logicalDevice;
		delete m_physicalDevice;
		delete m_backendInstance;
	}

	auto RenderContext::getBackendInstance() const -> gpu::VKInstance *
	{
		return m_backendInstance;
	}

	auto RenderContext::getPhysicalDevice() const -> gpu::VKPhysicalDevice *
	{
		return m_physicalDevice;
	}

	auto RenderContext::getLogicalDevice() const -> gpu::VKLogicalDevice *
	{
		return m_logicalDevice;
	}

	auto RenderContext::getGlobals() const -> const Globals *
	{
		return m_globals;
	}

	auto RenderContext::gpuWaitIdle() const -> void
	{
		m_logicalDevice->getVulkanLogicalDevice().waitIdle();
	}

	auto RenderContext::getCurrentFrameIndex() const -> uint32
	{
		return m_logicalDevice->getCurrentFrameIndex();
	}

	auto RenderContext::setCurrentFrameIndex(uint32 p_index) -> void
	{
		TST_PERMA_ASSERT_MSG(p_index < maxFramesInFlight, "Index is out of bounds!");
		m_logicalDevice->setCurrentFrameIndex(p_index);
	}

	auto RenderContext::performGarbageCollection() const -> void
	{
		m_logicalDevice->performGarbageCollection();
	}

	auto RenderContext::createAttachmentImage(uint32     p_width, uint32 p_height, vk::ImageAspectFlags p_image_aspect_flags,
											  vk::Format p_format) const -> gpu::RawImageHandle
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::ImageSpecInfo attachment_image_spec_info{};
		attachment_image_spec_info.width  = p_width;
		attachment_image_spec_info.height = p_height;
		attachment_image_spec_info.format = p_format;
		attachment_image_spec_info.usage  = gpu::util::getImageUsageFlags(p_image_aspect_flags);
		return createGPU<gpu::VKRawImage>(attachment_image_spec_info);
	}

	auto RenderContext::createMultisampleAttachmentImage(uint32     p_width, uint32 p_height, vk::ImageAspectFlags p_image_aspect_flags,
														 vk::Format p_format) const -> gpu::RawImageHandle
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::ImageSpecInfo attachment_image_spec_info{};
		attachment_image_spec_info.width       = p_width;
		attachment_image_spec_info.height      = p_height;
		attachment_image_spec_info.format      = p_format;
		attachment_image_spec_info.sampleCount = m_physicalDevice->getMaxUsableSampleCount();
		attachment_image_spec_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | gpu::util::getImageUsageFlags(p_image_aspect_flags);
		return createGPU<gpu::VKRawImage>(attachment_image_spec_info);
	}

	auto RenderContext::createAttachmentTexture(uint32     p_width, uint32 p_height, vk::ImageAspectFlags p_image_aspect_flags,
												vk::Format p_format) const -> gpu::Texture2DHandle
	{
		if (p_format == vk::Format::eUndefined)
			p_format = gpu::util::getDefaultFormat(p_image_aspect_flags);

		gpu::TextureSpecInfo attachment_texture_spec_info{};
		attachment_texture_spec_info.width        = p_width;
		attachment_texture_spec_info.height       = p_height;
		attachment_texture_spec_info.format       = p_format;
		attachment_texture_spec_info.generateMips = false;
		return createGPU<gpu::VKTexture2D>(attachment_texture_spec_info);
	}

	auto RenderContext::createEnvironmentMap(const io::filesystem::Path &p_path) const -> gpu::Texture3DHandle
	{
		auto env_tex{createGPU<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, p_path)};

		constexpr uint32     skybox_resolution{2048};
		gpu::TextureSpecInfo skybox_texture_map_spec_info{};
		skybox_texture_map_spec_info.width  = skybox_resolution;
		skybox_texture_map_spec_info.height = skybox_resolution;
		skybox_texture_map_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
		RefPtr<gpu::VKTexture3D> env_map    = createGPU<gpu::VKTexture3D>(skybox_texture_map_spec_info);

		auto equirectangular_to_cubemap_pipeline{createGPU<gpu::VKComputePipeline>(m_globals->shaderLibrary().get("Equirectangular_To_CubeMap"))};
		auto equirectangular_to_cubemap_pass{createGPU<gpu::VKComputePass>(equirectangular_to_cubemap_pipeline)};
		equirectangular_to_cubemap_pass->setInput("u_EquirectangularMap", env_tex);
		equirectangular_to_cubemap_pass->setInput("o_Cubemap", env_map);
		equirectangular_to_cubemap_pass->bake();

		gpu::VKCommandBuffer command_buffer{m_logicalDevice, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		beginCompute(&command_buffer, equirectangular_to_cubemap_pass, 0);
		dispatchCompute(&command_buffer, equirectangular_to_cubemap_pass, nullptr, skybox_resolution / 32, skybox_resolution / 32, 6, 0);

		command_buffer.end();
		command_buffer.submit();
		command_buffer.waitForFence();

		return env_map;
	}

	auto RenderContext::createEnvironmentMap(const gpu::TextureSpecInfo &p_spec_info, const Buffer &p_data) const -> gpu::Texture3DHandle
	{
		auto env_tex{createGPU<gpu::VKTexture2D>(p_spec_info, p_data)};

		constexpr uint32     skybox_resolution{2048};
		gpu::TextureSpecInfo skybox_texture_map_spec_info{};
		skybox_texture_map_spec_info.width  = skybox_resolution;
		skybox_texture_map_spec_info.height = skybox_resolution;
		skybox_texture_map_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
		RefPtr<gpu::VKTexture3D> env_map    = createGPU<gpu::VKTexture3D>(skybox_texture_map_spec_info);

		auto equirectangular_to_cubemap_pipeline{createGPU<gpu::VKComputePipeline>(m_globals->shaderLibrary().get("Equirectangular_To_CubeMap"))};
		auto equirectangular_to_cubemap_pass{createGPU<gpu::VKComputePass>(equirectangular_to_cubemap_pipeline)};
		equirectangular_to_cubemap_pass->setInput("u_EquirectangularMap", env_tex);
		equirectangular_to_cubemap_pass->setInput("o_Cubemap", env_map);
		equirectangular_to_cubemap_pass->bake();

		gpu::VKCommandBuffer command_buffer{m_logicalDevice, vk::QueueFlagBits::eCompute};
		command_buffer.begin();

		beginCompute(&command_buffer, equirectangular_to_cubemap_pass, 0);
		dispatchCompute(&command_buffer, equirectangular_to_cubemap_pass, nullptr, skybox_resolution / 32, skybox_resolution / 32, 6, 0);

		command_buffer.end();
		command_buffer.submit();
		command_buffer.waitForFence();

		return env_map;
	}

	auto RenderContext::beginRendering(gpu::VKCommandBuffer *p_command_buffer, const gpu::RenderingInfo &p_rendering_info, gpu::VKRenderPass *p_render_pass,
									   uint32                p_frame_index) const -> void
	{
		TST_ASSERT_MSG(p_render_pass, "Render pass is null");

		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};

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
				{
					gpu::util::transitionImageLayout(image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
													 p_command_buffer->getVulkanCommandBuffer());
				}

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
				{
					gpu::util::transitionImageLayout(resolve_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
													 p_command_buffer->getVulkanCommandBuffer());
				}

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
					gpu::util::transitionImageLayout(depth_image, vk::ImageLayout::eShaderReadOnlyOptimal,
													 p_rendering_info.depthReadOnly ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal,
													 p_command_buffer->getVulkanCommandBuffer());
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
					gpu::util::transitionImageLayout(depth_resolve_image, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthAttachmentOptimal,
													 p_command_buffer->getVulkanCommandBuffer());
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

		p_command_buffer->getVulkanCommandBuffer().beginRendering(rendering_info);
		p_command_buffer->getVulkanCommandBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipeline());
		p_command_buffer->getVulkanCommandBuffer().setViewport(0, viewport);
		p_command_buffer->getVulkanCommandBuffer().setScissor(0, scissor);

		p_render_pass->update(frame_index);

		const auto descriptor_sets = p_render_pass->getDescriptorSets(frame_index);
		if (!descriptor_sets.empty())
			p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_render_pass->getPipeline()->getPipelineLayout(),
																		  p_render_pass->getStartSetIndex(), descriptor_sets, nullptr);
	}

	auto RenderContext::endRendering(gpu::VKCommandBuffer *p_command_buffer, const gpu::RenderingInfo &p_rendering_info) const -> void
	{
		p_command_buffer->getVulkanCommandBuffer().endRendering();

		// Perform the layout transition on sampled attachment images
		for (const auto &rendering_attachment: p_rendering_info.colourAttachments)
		{
			auto image{rendering_attachment.image};
			if ((image != nullptr) && (image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 p_command_buffer->getVulkanCommandBuffer());
			}
			auto resolve_image{rendering_attachment.resolveImage};
			if ((resolve_image != nullptr) && (resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(resolve_image, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 p_command_buffer->getVulkanCommandBuffer());
			}
		}

		if (p_rendering_info.pDepthAttachment != nullptr)
		{
			auto depth_image{p_rendering_info.pDepthAttachment->image};

			if ((depth_image != nullptr) && (depth_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(depth_image,
												 p_rendering_info.depthReadOnly ? vk::ImageLayout::eDepthReadOnlyOptimal : vk::ImageLayout::eDepthAttachmentOptimal,
												 vk::ImageLayout::eShaderReadOnlyOptimal, p_command_buffer->getVulkanCommandBuffer());
			}
			auto depth_resolve_image{p_rendering_info.pDepthAttachment->resolveImage};
			if ((depth_resolve_image != nullptr) && (depth_resolve_image->getSpecInfo().usage & vk::ImageUsageFlagBits::eSampled))
			{
				gpu::util::transitionImageLayout(depth_resolve_image, vk::ImageLayout::eDepthAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
												 p_command_buffer->getVulkanCommandBuffer());
			}
		}
	}

	auto RenderContext::beginCompute(gpu::VKCommandBuffer *p_command_buffer, gpu::VKComputePass *p_compute_pass, uint32 p_frame_index) const -> void
	{
		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};

		p_command_buffer->getVulkanCommandBuffer().bindPipeline(vk::PipelineBindPoint::eCompute, *p_compute_pass->getPipeline());

		p_compute_pass->update(frame_index);

		const auto descriptor_sets = p_compute_pass->getDescriptorSets(frame_index);
		if (!descriptor_sets.empty())
			p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipelineLayout(),
																		  p_compute_pass->getStartSetIndex(), descriptor_sets, nullptr);
	}

	auto RenderContext::dispatchCompute(gpu::VKCommandBuffer *p_command_buffer, const gpu::VKComputePass *p_compute_pass, Material *p_material, uint32 p_work_group_x,
										uint32                p_work_group_y, uint32                      p_work_group_z, uint32    p_frame_index) const -> void
	{
		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};

		if (p_material)
			if (p_material->hasDescriptorSets())
				if (const auto descriptor_set{p_material->getDescriptorSet(frame_index)})
					p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, p_compute_pass->getPipeline()->getPipelineLayout(), 0,
																				  descriptor_set, nullptr);

		p_command_buffer->getVulkanCommandBuffer().dispatch(p_work_group_x, p_work_group_y, p_work_group_z);
	}

	auto RenderContext::renderGeometry(gpu::VKCommandBuffer *p_command_buffer, gpu::VKPipeline *p_pipeline, gpu::VKVertexBuffer *p_vertex_buffer,
									   gpu::VKIndexBuffer *  p_index_buffer, uint32             p_index_count, Material *        p_material, const glm::mat4 &p_transform,
									   uint32                p_frame_index) const -> void
	{
		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};

		// Push the constants
		p_command_buffer->getVulkanCommandBuffer().pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		if (p_material)
		{
			if (p_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(frame_index)};
				p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0,
																			  material_descriptor_set, {});
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

				p_command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_vertex_buffer->bind(p_command_buffer->getVulkanCommandBuffer());
		p_index_buffer->bind(p_command_buffer->getVulkanCommandBuffer(), vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer->getVulkanCommandBuffer().drawIndexed(p_index_count, 1, 0, 0, 0);
	}

	auto RenderContext::renderFullscreenQuad(gpu::VKCommandBuffer *p_command_buffer, gpu::VKPipeline *p_pipeline, Material *p_material,
											 uint32                p_frame_index) const -> void
	{
		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};

		if (p_material) // You technically don't need to use a material if you don't want to
		{
			if (p_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_material->getDescriptorSet(frame_index)};
				p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0,
																			  material_descriptor_set, {});
			}

			const auto &push_constants{p_material->getPushConstantStorageBuffer()};
			if (push_constants.size() > 0)
			{
				vk::PushConstantsInfo push_constants_info{};
				push_constants_info.layout     = p_pipeline->getPipelineLayout();
				push_constants_info.stageFlags = vk::ShaderStageFlagBits::eFragment;
				push_constants_info.size       = push_constants.size();
				push_constants_info.offset     = 0u;
				push_constants_info.pValues    = push_constants.data();
				p_command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		m_globals->fullscreenQuadVertexBuffer()->bind(p_command_buffer->getVulkanCommandBuffer());
		m_globals->fullscreenQuadIndexBuffer()->bind(p_command_buffer->getVulkanCommandBuffer(), vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer->getVulkanCommandBuffer().drawIndexed(m_globals->fullscreenQuadIndices().size(), 1, 0, 0, 0);
	}

	auto RenderContext::renderMesh(gpu::VKCommandBuffer *p_command_buffer, const MeshData *p_mesh, uint32 p_submesh_index, gpu::VKPipeline *p_pipeline,
								   const glm::mat4 &     p_transform, uint32               p_frame_index) const -> void
	{
		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};

		// Push the constants
		p_command_buffer->getVulkanCommandBuffer().pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		const auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};
		auto        material{p_mesh->getMaterials().getMaterial(submesh.materialIndex).material};

		if (material) // You technically don't need to use a material if you don't want to
		{
			if (material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{material->getDescriptorSet(frame_index)};
				p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0,
																			  material_descriptor_set, {});
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

				p_command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_mesh->getVertexBuffer()->bind(p_command_buffer->getVulkanCommandBuffer());
		p_mesh->getIndexBuffer()->bind(p_command_buffer->getVulkanCommandBuffer(), vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer->getVulkanCommandBuffer().drawIndexed(submesh.indexCount, 1, submesh.baseIndex, submesh.baseVertex, 0);
	}

	auto RenderContext::renderMesh(gpu::VKCommandBuffer *p_command_buffer, const MeshData *p_mesh, uint32              p_submesh_index, gpu::VKPipeline *p_pipeline,
								   const glm::mat4 &     p_transform, Material *           p_override_material, uint32 p_frame_index) const -> void
	{
		uint32 frame_index{(p_frame_index == UINT32_MAX) ? getCurrentFrameIndex() : p_frame_index};
		// Push the constants
		p_command_buffer->getVulkanCommandBuffer().pushConstants<glm::mat4>(p_pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, p_transform);

		const auto &submesh{p_mesh->getSubmeshes()[p_submesh_index]};

		if (p_override_material) // You technically don't need to use a material if you don't want to
		{
			if (p_override_material->hasDescriptorSets())
			{
				// Bind the material descriptor set (0)
				vk::DescriptorSet material_descriptor_set{p_override_material->getDescriptorSet(frame_index)};
				p_command_buffer->getVulkanCommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, p_pipeline->getPipelineLayout(), 0,
																			  material_descriptor_set, {});
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

				p_command_buffer->getVulkanCommandBuffer().pushConstants2(push_constants_info);
			}
		}
		// Bind the vertex and index buffers
		p_mesh->getVertexBuffer()->bind(p_command_buffer->getVulkanCommandBuffer());
		p_mesh->getIndexBuffer()->bind(p_command_buffer->getVulkanCommandBuffer(), vk::IndexType::eUint32);

		// Finally, draw indexed :)
		p_command_buffer->getVulkanCommandBuffer().drawIndexed(submesh.indexCount, 1, submesh.baseIndex, static_cast<int32>(submesh.baseVertex), 0);
	}
}
