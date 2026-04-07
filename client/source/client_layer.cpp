#include "client_layer.hpp"

#include "stb/stb_image.h"
#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toast_lib/logging.hpp"

#include "toast_gpu/vk/vk_swapchain.hpp"

#include <imgui.h>
namespace ig = ImGui;

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ClientLayer::onInit()
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		// auto geometry_pass = m_renderGraph.addPass("Geometry").
		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		swapchain->addResizeCallback([this](uint32 width, uint32 height)
		{
			LOG_INFO("{}, {}", width, height);

			m_viewportWidth  = width;
			m_viewportHeight = height;

			// _createAttachmentImages();
		});

		{
			m_geometryVertexBufferLayout = {
				{gpu::EShaderDataType::eFloat3, "a_Position"},
				{gpu::EShaderDataType::eFloat3, "a_Normal"},
				{gpu::EShaderDataType::eFloat3, "a_Tangent"},
				{gpu::EShaderDataType::eFloat3, "a_Bitangent"},
				{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
			};

			gpu::VKShader::Bytecode    vs_bytecode = io::filesystem::readBinary("shaders/geometry.vert.glsl.spv");
			gpu::VKShader::Bytecode    ps_bytecode = io::filesystem::readBinary("shaders/geometry.pixel.glsl.spv");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
			m_geometryShader = make_reference<gpu::VKShader>(ctx, shader_bytecode_map);

			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = m_geometryVertexBufferLayout;
			pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
			pipeline_create_info.depthFormat        = {swapchain->getDepthFormat()};
			pipeline_create_info.shader             = m_geometryShader;
			m_geometryPipeline                      = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

			gpu::TextureSpecInfo texture_spec_info{};
			m_texture = make_reference<gpu::VKTexture2D>(ctx, texture_spec_info, "../resources/textures/Peeber.png");

			constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
			m_ubos                 = make_reference<gpu::VKUniformBufferPFF>(ctx, ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedUniformBuffers = m_ubos->mapMemory(ubo_size, 0);
		}

		m_mesh = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/Orbo.fbx");

		// _createAttachmentImages();
		_createDescriptorPool();
		_createDescriptorSets();
	}

	void ClientLayer::onDestroy()
	{
		m_ubos->unmapMemory();

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		ctx->getDevice().waitIdle();
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		uint32 image_index{swapchain->getImageIndex()};

		vk::Extent2D swapchain_extent{swapchain->getExtent()};
		auto &       command_buffer = swapchain->getCurrentCommandBuffer();

		CameraUB camera_ub{};
		camera_ub.model = glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{20.0f, 20.0f, 20.0f}), m_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
		// camera_ub.view  = m_editorCamera.getViewMatrix();
		// camera_ub.proj  = m_editorCamera.getProjectionMatrix();
		camera_ub.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
		camera_ub.proj = glm::perspective(glm::radians(45.0f), static_cast<float32>(swapchain_extent.width) / static_cast<float32>(swapchain_extent.height), 0.1f, 10.0f);
		camera_ub.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedUniformBuffers[frame_index], &camera_ub, sizeof(CameraUB));

		#if 0
		if (m_colourAttachmentLayout != vk::ImageLayout::eColorAttachmentOptimal)
		{
			ctx->transitionImageLayout(m_colourAttachmentImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
									   vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eFragmentShader,
									   vk::PipelineStageFlagBits::eColorAttachmentOutput, 1);
		}
		{
			vk::ClearValue              clear_colour = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
			vk::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue  = clear_colour;
			colour_attachment_info.imageView   = m_colourAttachmentImageView;
			colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

			vk::RenderingInfo rendering_info{};
			rendering_info.renderArea           = vk::Rect2D{{0, 0}, swapchain_extent};
			rendering_info.layerCount           = 1;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments    = &colour_attachment_info;
			rendering_info.pDepthAttachment     = nullptr;

			vk::Viewport viewport{};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			viewport.x        = 0.0f;
			viewport.y        = 0.0f;
			viewport.width    = static_cast<float32>(swapchain_extent.width);
			viewport.height   = static_cast<float32>(swapchain_extent.height);

			vk::Rect2D scissor{vk::Offset2D{0, 0}, swapchain_extent};

			command_buffer.beginRendering(rendering_info);

			command_buffer.endRendering();

			ctx->transitionImageLayout(m_colourAttachmentImage, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
									   vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eColorAttachmentOutput,
									   vk::PipelineStageFlagBits::eFragmentShader, 1);
			m_colourAttachmentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}
		#endif
		{
			vk::ClearValue              clear_colour = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
			vk::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue  = clear_colour;
			colour_attachment_info.imageView   = swapchain->getImageView(image_index);
			colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

			vk::ClearValue              clear_depth = vk::ClearDepthStencilValue{1.0f, 0u};
			vk::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue  = clear_depth;
			depth_attachment_info.imageView   = swapchain->getDepthImageView();
			depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

			vk::RenderingInfo rendering_info{};
			rendering_info.renderArea           = vk::Rect2D{{0, 0}, swapchain_extent};
			rendering_info.layerCount           = 1;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments    = &colour_attachment_info;
			rendering_info.pDepthAttachment     = &depth_attachment_info;

			vk::Viewport viewport{};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			viewport.x        = 0.0f;
			viewport.y        = 0.0f;
			viewport.width    = static_cast<float32>(swapchain_extent.width);
			viewport.height   = static_cast<float32>(swapchain_extent.height);

			vk::Rect2D scissor{vk::Offset2D{0, 0}, swapchain_extent};

			command_buffer.beginRendering(rendering_info);
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_geometryPipeline->getPipeline());
			command_buffer.setViewport(0, viewport);
			command_buffer.setScissor(0, scissor);

			MaterialCB material_constant_buffer{};
			material_constant_buffer.roughness = m_mesh->getRoughness();
			command_buffer.pushConstants<MaterialCB>(m_geometryPipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, material_constant_buffer);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_geometryPipeline->getPipelineLayout(), 0, *m_descriptorSets[frame_index], nullptr);

			m_mesh->getVertexBuffer()->bind(command_buffer);
			m_mesh->getIndexBuffer()->bind(command_buffer, vk::IndexType::eUint16);

			command_buffer.drawIndexed(m_mesh->getIndices().size(), 1, 0, 0, 0);
			command_buffer.endRendering();
		}
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(ClientLayer::onWindowResizeEvent));
	}

	void ClientLayer::onUIRender()
	{
		// ig::Begin("Viewport");
		// ig::End();
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
			getApp().close();

		return false;
	}

	bool ClientLayer::onWindowResizeEvent(WindowResizeEvent &e)
	{
		return false;
	}

	void ClientLayer::_createAttachmentImages()
	{
		m_colourAttachmentImage               = nullptr;
		m_colourAttachmentImageMemory         = nullptr;
		m_colourAttachmentImageView           = nullptr;
		m_colourAttachmentImageDescriptorInfo = vk::DescriptorImageInfo{};

		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain{app.getWindow().getSwapchain()};

		vk::Format colour_attachment_format{swapchain->getSurfaceFormat().format};
		ctx->createImage(m_viewportWidth, m_viewportHeight, 1, colour_attachment_format, vk::ImageTiling::eOptimal,
						 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_colourAttachmentImage,
						 m_colourAttachmentImageMemory);

		if (m_colourAttachmentLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			ctx->transitionImageLayout(m_colourAttachmentImage, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal,
									   vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eFragmentShader,
									   vk::PipelineStageFlagBits::eColorAttachmentOutput, 1);
		}
		else
		{
			ctx->transitionImageLayout(m_colourAttachmentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone,
									   vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eTopOfPipe,
									   vk::PipelineStageFlagBits::eColorAttachmentOutput, 1);
		}
		m_colourAttachmentLayout = vk::ImageLayout::eColorAttachmentOptimal;

		m_colourAttachmentImageView = ctx->createImageView(m_colourAttachmentImage, colour_attachment_format, vk::ImageAspectFlagBits::eColor, 1);

		auto physical_device_props{ctx->getPhysicalDevice().getProperties()};

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = vk::LodClampNone;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatCustomEXT;
		sampler_create_info.unnormalizedCoordinates = false;

		// This is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_colourAttachmentImageSampler = {ctx->getDevice(), sampler_create_info};

		m_colourAttachmentImageDescriptorInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		m_colourAttachmentImageDescriptorInfo.imageView   = m_colourAttachmentImageView;
		m_colourAttachmentImageDescriptorInfo.sampler     = m_colourAttachmentImageSampler;
	}

	void ClientLayer::_createDescriptorPool()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		std::array<vk::DescriptorPoolSize, 2> descriptor_pool_sizes{};
		descriptor_pool_sizes[0].descriptorCount = 2 * gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[0].type            = vk::DescriptorType::eUniformBuffer;

		descriptor_pool_sizes[1].descriptorCount = 2 * gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[1].type            = vk::DescriptorType::eCombinedImageSampler;

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
		descriptor_pool_create_info.pPoolSizes    = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets       = gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_create_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		m_descriptorPool = {ctx->getDevice(), descriptor_pool_create_info};
	}

	void ClientLayer::_createDescriptorSets()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		{
			auto &                        set_layout = m_geometryShader->getDescriptorSetLayout(0);
			std::vector                   descriptor_set_layouts{gpu::VKGPUContext::c_maxFramesInFlight, *set_layout};
			vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
			descriptor_set_allocate_info.descriptorSetCount = gpu::VKGPUContext::c_maxFramesInFlight;
			descriptor_set_allocate_info.pSetLayouts        = descriptor_set_layouts.data();

			m_descriptorSets = ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info);

			for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
			{
				std::array<vk::WriteDescriptorSet, 3> write_descriptor_sets{};
				write_descriptor_sets[0].descriptorCount = 1;
				write_descriptor_sets[0].descriptorType  = vk::DescriptorType::eUniformBuffer;
				write_descriptor_sets[0].pBufferInfo     = &m_ubos->getUBO(i)->getDescriptorInfo();
				write_descriptor_sets[0].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[0].dstBinding      = 0;
				write_descriptor_sets[0].dstArrayElement = 0;

				write_descriptor_sets[1].descriptorCount = 1;
				write_descriptor_sets[1].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_sets[1].pImageInfo      = &m_mesh->getAlbedoMap()->getDescriptorInfo();
				write_descriptor_sets[1].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[1].dstBinding      = 1;
				write_descriptor_sets[1].dstArrayElement = 0;

				write_descriptor_sets[2].descriptorCount = 1;
				write_descriptor_sets[2].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_sets[2].pImageInfo      = &m_texture->getDescriptorInfo();
				write_descriptor_sets[2].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[2].dstBinding      = 2;
				write_descriptor_sets[2].dstArrayElement = 0;

				ctx->getDevice().updateDescriptorSets(write_descriptor_sets, {});
			}
		}
	}
}
