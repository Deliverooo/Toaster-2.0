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

		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		m_viewportWidth  = window_width;
		m_viewportHeight = window_height;

		swapchain->addResizeCallback([this](uint32 width, uint32 height)
		{
			LOG_INFO("{}, {}", width, height);

			m_viewportWidth  = width;
			m_viewportHeight = height;

			_createAttachmentImages();
		});

		{
			gpu::VKShader::Bytecode    vs_bytecode = io::filesystem::readBinary("shaders/geometry.vert.glsl.spv");
			gpu::VKShader::Bytecode    ps_bytecode = io::filesystem::readBinary("shaders/geometry.pixel.glsl.spv");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
			m_geometryShader = make_reference<gpu::VKShader>(ctx, shader_bytecode_map);

			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {
				{gpu::EShaderDataType::eFloat3, "a_Position"},
				{gpu::EShaderDataType::eFloat3, "a_Normal"},
				{gpu::EShaderDataType::eFloat3, "a_Tangent"},
				{gpu::EShaderDataType::eFloat3, "a_Bitangent"},
				{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
			};;
			pipeline_create_info.colourAttachments = {swapchain->getSurfaceFormat().format};
			pipeline_create_info.depthFormat       = {swapchain->getDepthFormat()};
			pipeline_create_info.shader            = m_geometryShader;
			m_geometryPipeline                     = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

			constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
			m_ubos                 = make_reference<gpu::VKUniformBufferPFF>(ctx, ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedUniformBuffers = m_ubos->mapMemory(ubo_size, 0);

			m_geometryPass = make_reference<gpu::VKRenderPass>(ctx, m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_ubos);
			m_geometryPass->bake(); // TODO: rename ts to toast
		}

		gpu::TextureSpecInfo texture_spec_info{};
		m_texture = make_reference<gpu::VKTexture2D>(ctx, texture_spec_info, "../resources/textures/Peeber.png");

		m_mesh = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/Orbo.fbx");

		_createAttachmentImages();
		_createDescriptorPool();
		// _createDescriptorSets();
	}

	void ClientLayer::onDestroy()
	{
		m_ubos->unmapMemory();

		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
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
		camera_ub.view = glm::lookAt(glm::vec3{2.0f, 2.0f, 2.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
		camera_ub.proj = glm::perspective(glm::radians(45.0f), static_cast<float32>(swapchain_extent.width) / static_cast<float32>(swapchain_extent.height), 0.1f, 10.0f);
		camera_ub.proj[1][1] *= -1.0f;

		std::memcpy(m_mappedUniformBuffers[frame_index], &camera_ub, sizeof(CameraUB));

		{
			vk::ClearValue              clear_colour = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
			vk::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue         = clear_colour;
			colour_attachment_info.imageView          = m_colourAttachmentImageView;
			colour_attachment_info.imageLayout        = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			colour_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eAverage;
			colour_attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.resolveImageView   = swapchain->getImageView(swapchain->getImageIndex());

			vk::ClearValue              clear_depth = vk::ClearDepthStencilValue{1.0f, 0u};
			vk::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue         = clear_depth;
			depth_attachment_info.imageView          = m_depthAttachmentImageView;
			depth_attachment_info.imageLayout        = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eMin;
			depth_attachment_info.resolveImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.resolveImageView   = swapchain->getDepthImageView();

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

			TransformCB transform_constant_buffer{};
			transform_constant_buffer.model = glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{20.0f, 20.0f, 20.0f}), m_time * glm::radians(90.0f),
														  glm::vec3{0.0f, 0.0f, 1.0f});
			command_buffer.pushConstants<TransformCB>(m_geometryPipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, transform_constant_buffer);

			MaterialCB material_constant_buffer{};
			material_constant_buffer.roughness = m_mesh->getRoughness();
			command_buffer.pushConstants<MaterialCB>(m_geometryPipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eFragment, sizeof(TransformCB),
													 material_constant_buffer);

			const auto descriptor_sets=  m_geometryPass->getDescriptorSets(frame_index);
			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_geometryPipeline->getPipelineLayout(), 0,descriptor_sets, nullptr);

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
		m_colourAttachmentImage       = nullptr;
		m_colourAttachmentImageMemory = nullptr;
		m_colourAttachmentImageView   = nullptr;

		m_depthAttachmentImage       = nullptr;
		m_depthAttachmentImageMemory = nullptr;
		m_depthAttachmentImageView   = nullptr;

		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain{app.getWindow().getSwapchain()};

		vk::SampleCountFlagBits sample_count{ctx->getMaxUsableSampleCount()};

		{
			vk::Format colour_attachment_format{swapchain->getSurfaceFormat().format};
			ctx->createImage(m_viewportWidth, m_viewportHeight, 1, sample_count, colour_attachment_format, vk::ImageTiling::eOptimal,
							 vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
							 m_colourAttachmentImage, m_colourAttachmentImageMemory);

			ctx->transitionImageLayout(m_colourAttachmentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone,
									   vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput, 1,
									   vk::ImageAspectFlagBits::eColor);

			m_colourAttachmentImageView = ctx->createImageView(m_colourAttachmentImage, colour_attachment_format, vk::ImageAspectFlagBits::eColor, 1);
		}
		{
			vk::Format depth_attachment_format{swapchain->getDepthFormat()};
			ctx->createImage(m_viewportWidth, m_viewportHeight, 1, sample_count, depth_attachment_format, vk::ImageTiling::eOptimal,
							 vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
							 m_depthAttachmentImage, m_depthAttachmentImageMemory);

			ctx->transitionImageLayout(m_depthAttachmentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits::eNone,
									   vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits::eNone,
									   vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, 1,
									   vk::ImageAspectFlagBits::eDepth);

			m_depthAttachmentImageView = ctx->createImageView(m_depthAttachmentImage, depth_attachment_format, vk::ImageAspectFlagBits::eDepth, 1);
		}
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
