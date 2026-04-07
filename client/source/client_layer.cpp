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

#include "glm/gtc/type_ptr.hpp"
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

			m_renderer2D->onResize(width, height);
			_createDescriptorSets();
		});

		{
			m_compositeVertexBufferLayout = {{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoord"}};

			gpu::VKShader::Bytecode    vs_bytecode = io::filesystem::readBinary("shaders/composite.vert.glsl.spv");
			gpu::VKShader::Bytecode    ps_bytecode = io::filesystem::readBinary("shaders/composite.pixel.glsl.spv");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
			m_compositeShader = make_reference<gpu::VKShader>(ctx, shader_bytecode_map);

			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = m_compositeVertexBufferLayout;
			pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
			pipeline_create_info.depthFormat        = {swapchain->getDepthFormat()};
			pipeline_create_info.shader             = m_compositeShader;
			m_compositePipeline                     = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}});
			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}});
			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}});
			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}});
			m_fullscreenQuadIndices = {0, 1, 3, 1, 2, 3};

			vk::DeviceSize vbo_size{m_fullscreenQuadVertices.size() * sizeof(FullscreenQuadVertex)};
			m_fullscreenQuadVertexBuffer = make_reference<gpu::VKVertexBuffer>(ctx, m_fullscreenQuadVertices.data(), vbo_size);

			vk::DeviceSize ibo_size{m_fullscreenQuadIndices.size() * sizeof(uint16)};
			m_fullscreenQuadIndexBuffer = make_reference<gpu::VKIndexBuffer>(ctx, m_fullscreenQuadIndices.data(), ibo_size);

			gpu::TextureSpecInfo texture_spec_info{};
			m_texture = make_reference<gpu::VKTexture2D>(ctx, texture_spec_info, "../resources/textures/Peeber.png");
		}

		// m_mesh = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/Orbo.fbx");

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = m_viewportWidth;
		renderer_2d_create_info.renderTargetHeight = m_viewportHeight;
		m_renderer2D                               = make_reference<Renderer2D>(ctx, renderer_2d_create_info);

		_createAttachmentImages();
		_createDescriptorPool();
		_createDescriptorSets();
	}

	void ClientLayer::onDestroy()
	{
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

		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		// Renderer2D needs orthographic projection for 2D rendering
		glm::mat4 ortho_view = glm::mat4(1.0f);
		glm::mat4 ortho_proj = glm::ortho(
			0.0f, static_cast<float32>(m_viewportWidth),
			static_cast<float32>(m_viewportHeight), 0.0f,
			-1.0f, 1.0f
		);
		// Flip Y axis for Vulkan NDC
		ortho_proj[1][1] *= -1.0f;

		m_renderer2D->begin(command_buffer, frame_index, ortho_view, ortho_proj);
		// Submit quad at screen coordinates (100, 100) with size (200, 200)
		glm::mat4 quad_transform = glm::translate(glm::mat4{1.0f}, glm::vec3{100.0f, 100.0f, 0.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{200.0f, 200.0f, 1.0f});
		m_renderer2D->submitQuad(quad_transform, {1.0f, 1.0f, 1.0f, 1.0f});
		m_renderer2D->end(command_buffer, frame_index);

		{
			vk::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue         = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};;
			colour_attachment_info.imageView          = m_colourAttachmentImageView;
			colour_attachment_info.imageLayout        = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			colour_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eAverage;
			colour_attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.resolveImageView   = swapchain->getImageView(image_index);

			vk::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue         = vk::ClearDepthStencilValue{1.0f, 0u};;
			depth_attachment_info.imageView          = m_depthAttachmentImageView;
			depth_attachment_info.imageLayout        = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eMin;
			depth_attachment_info.resolveImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.resolveImageView   = swapchain->getDepthImageView();

			vk::RenderingInfo rendering_info{};
			rendering_info.renderArea           = vk::Rect2D{{0, 0}, vk::Extent2D{m_viewportWidth, m_viewportHeight}};
			rendering_info.layerCount           = 1;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments    = &colour_attachment_info;
			rendering_info.pDepthAttachment     = &depth_attachment_info;

			vk::Viewport viewport{0.0f, 0.0f, static_cast<float32>(m_viewportWidth), static_cast<float32>(m_viewportHeight), 0.0f, 1.0f};
			vk::Rect2D   scissor{vk::Offset2D{0, 0}, vk::Extent2D{m_viewportWidth, m_viewportHeight}};

			command_buffer.beginRendering(rendering_info);
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_compositePipeline->getPipeline());
			command_buffer.setViewport(0, viewport);
			command_buffer.setScissor(0, scissor);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_compositePipeline->getPipelineLayout(), 0, *m_descriptorSets[frame_index], nullptr);
			m_fullscreenQuadVertexBuffer->bind(command_buffer);
			m_fullscreenQuadIndexBuffer->bind(command_buffer, vk::IndexType::eUint16);

			command_buffer.drawIndexed(m_fullscreenQuadIndices.size(), 1, 0, 0, 0);
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
		ig::Begin("Viewport");

		ig::SliderFloat3("Translation", glm::value_ptr(m_quadTranslation), -10.0f, 10.0f);
		ig::SliderFloat3("Scale", glm::value_ptr(m_quadScale), -10.0f, 10.0f);
		ig::End();
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

		m_descriptorSets.clear();
		{
			auto &                        set_layout = m_compositeShader->getDescriptorSetLayout(0);
			std::vector                   descriptor_set_layouts{gpu::VKGPUContext::c_maxFramesInFlight, *set_layout};
			vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
			descriptor_set_allocate_info.descriptorSetCount = gpu::VKGPUContext::c_maxFramesInFlight;
			descriptor_set_allocate_info.pSetLayouts        = descriptor_set_layouts.data();

			m_descriptorSets = ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info);

			for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
			{
				std::array<vk::WriteDescriptorSet, 1> write_descriptor_sets{};
				write_descriptor_sets[0].descriptorCount = 1;
				write_descriptor_sets[0].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_sets[0].pImageInfo      = &m_renderer2D->getRenderTargetDescriptorImageInfo();
				write_descriptor_sets[0].dstSet          = m_descriptorSets[i];
				write_descriptor_sets[0].dstBinding      = 0;
				write_descriptor_sets[0].dstArrayElement = 0;

				ctx->getDevice().updateDescriptorSets(write_descriptor_sets, {});
			}
		}
	}
}
