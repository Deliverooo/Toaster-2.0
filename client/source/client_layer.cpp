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

			m_MSAAColourAttachmentImage->resize(width, height);
			m_MSAADepthAttachmentImage->resize(width, height);

			m_geometryColourAttachmentTexture->resize(width, height);
			m_geometryDepthAttachmentImage->resize(width, height);

			m_renderer2D->onResize(width, height);
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
			pipeline_create_info.multisample       = false;
			m_geometryPipeline                     = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

			constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
			m_ubos                 = make_reference<gpu::VKUniformBufferPFF>(ctx, ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedUniformBuffers = m_ubos->mapMemory(ubo_size, 0);

			m_geometryPass = make_reference<gpu::VKRenderPass>(ctx, m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_ubos);

			m_geometryPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...

			gpu::ImageCreateInfo colour_attachment_image_create_info{};
			colour_attachment_image_create_info.width       = window_width;
			colour_attachment_image_create_info.height      = window_height;
			colour_attachment_image_create_info.format      = swapchain->getSurfaceFormat().format;
			colour_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
			colour_attachment_image_create_info.sampleCount = ctx->getMaxUsableSampleCount();
			m_MSAAColourAttachmentImage                     = make_reference<gpu::VKImage2D>(ctx, colour_attachment_image_create_info);

			gpu::ImageCreateInfo depth_attachment_image_create_info{};
			depth_attachment_image_create_info.width       = window_width;
			depth_attachment_image_create_info.height      = window_height;
			depth_attachment_image_create_info.format      = swapchain->getDepthFormat();
			depth_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
			depth_attachment_image_create_info.sampleCount = ctx->getMaxUsableSampleCount();
			m_MSAADepthAttachmentImage                     = make_reference<gpu::VKImage2D>(ctx, depth_attachment_image_create_info);
		}
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

			m_fullscreenPass = make_reference<gpu::VKRenderPass>(ctx, m_compositePipeline);
			m_fullscreenPass->bake();

			m_fullscreenMaterial = make_reference<gpu::VKMaterial>(ctx, m_compositeShader);

			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
			m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});
			m_fullscreenQuadIndices = {0, 1, 3, 1, 2, 3};

			vk::DeviceSize vbo_size{m_fullscreenQuadVertices.size() * sizeof(FullscreenQuadVertex)};
			m_fullscreenQuadVertexBuffer = make_reference<gpu::VKVertexBuffer>(ctx, m_fullscreenQuadVertices.data(), vbo_size);

			vk::DeviceSize ibo_size{m_fullscreenQuadIndices.size() * sizeof(uint16)};
			m_fullscreenQuadIndexBuffer = make_reference<gpu::VKIndexBuffer>(ctx, m_fullscreenQuadIndices.data(), ibo_size);

			gpu::TextureSpecInfo colour_attachment_texture_spec_info{};
			colour_attachment_texture_spec_info.width  = window_width;
			colour_attachment_texture_spec_info.height = window_height;
			colour_attachment_texture_spec_info.format = swapchain->getSurfaceFormat().format;
			m_geometryColourAttachmentTexture          = make_reference<gpu::VKTexture2D>(ctx, colour_attachment_texture_spec_info);

			gpu::ImageCreateInfo depth_attachment_image_create_info{};
			depth_attachment_image_create_info.width  = window_width;
			depth_attachment_image_create_info.height = window_height;
			depth_attachment_image_create_info.format = swapchain->getDepthFormat();
			depth_attachment_image_create_info.usage  = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
			m_geometryDepthAttachmentImage            = make_reference<gpu::VKImage2D>(ctx, depth_attachment_image_create_info);
		}

		m_mesh  = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/Orbo.fbx", m_geometryShader);
		m_mesh2 = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/DJT_sculpt.fbx", m_geometryShader);

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = window_width;
		renderer_2d_create_info.renderTargetHeight = window_height;
		m_renderer2D                               = make_reference<Renderer2D>(ctx, renderer_2d_create_info);
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

		m_renderer2D->begin(command_buffer, frame_index, camera_ub.view, camera_ub.proj);
		m_renderer2D->submitQuad(m_meshTranslation, glm::vec2{109.0f, 109.0f}, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f});
		m_renderer2D->end(command_buffer, frame_index);

		m_geometryPass->setInput("u_2D", m_renderer2D->getColourOutput());

		std::memcpy(m_mappedUniformBuffers[frame_index], &camera_ub, sizeof(CameraUB));

		{
			gpu::RenderingInfo rendering_info{};
			rendering_info.renderArea = vk::Rect2D{{0, 0}, swapchain_extent};
			rendering_info.layerCount = 1;

			gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.clearValue = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
			colour_attachment_info.image      = m_geometryColourAttachmentTexture->getImage();
			colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
			depth_attachment_info.image      = m_geometryDepthAttachmentImage;
			depth_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;
			rendering_info.pDepthAttachment  = &depth_attachment_info;

			Renderer::beginRendering(rendering_info, command_buffer, frame_index, m_geometryPass);
			glm::mat4 transform{glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{20.0f, 20.0f, 20.0f}), m_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f})};
			glm::mat4 transform2{
				glm::translate(glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{10.0f, 10.0f, 10.0f}), m_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f}),
							   m_meshTranslation)
			};

			Renderer::renderGeometry(command_buffer, frame_index, m_geometryPipeline, m_mesh->getVertexBuffer(), m_mesh->getIndexBuffer(), m_mesh->getIndices().size(),
									 m_mesh->getMaterial(), transform);
			Renderer::renderGeometry(command_buffer, frame_index, m_geometryPipeline, m_mesh2->getVertexBuffer(), m_mesh2->getIndexBuffer(), m_mesh2->getIndices().size(),
									 m_mesh2->getMaterial(), transform2);
			Renderer::endRendering(rendering_info, command_buffer);
		}
		{
			m_fullscreenPass->setInput("u_Texture", m_geometryColourAttachmentTexture);

			gpu::RenderingInfo rendering_info{};
			rendering_info.renderArea = vk::Rect2D{{0, 0}, swapchain_extent};
			rendering_info.layerCount = 1;

			gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.clearValue         = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
			colour_attachment_info.image              = m_MSAAColourAttachmentImage;
			colour_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			colour_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eAverage;
			colour_attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.resolveImageView   = swapchain->getImageView(image_index);

			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue         = vk::ClearDepthStencilValue{1.0f, 0u};
			depth_attachment_info.image              = m_MSAADepthAttachmentImage;
			depth_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eMin;
			depth_attachment_info.resolveImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.resolveImageView   = swapchain->getDepthImageView();
			rendering_info.pDepthAttachment          = &depth_attachment_info;

			Renderer::beginRendering(rendering_info, command_buffer, frame_index, m_fullscreenPass);
			Renderer::renderGeometry(command_buffer, frame_index, m_fullscreenPass->getPipeline(), m_fullscreenQuadVertexBuffer, m_fullscreenQuadIndexBuffer,
									 m_fullscreenQuadIndices.size(), m_fullscreenMaterial, glm::mat4{1.0f});
			Renderer::endRendering(rendering_info, command_buffer);
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
		ig::Begin("Tools");

		ig::SliderFloat3("Translation", glm::value_ptr(m_meshTranslation), -1.0f, 1.0f);

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
}
