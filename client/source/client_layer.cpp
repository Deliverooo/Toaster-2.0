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

			m_colourAttachmentImage->resize(width, height);
			m_depthAttachmentImage->resize(width, height);
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

			gpu::TextureSpecInfo texture_spec_info{};
			m_texture = make_reference<gpu::VKTexture2D>(ctx, texture_spec_info, "../resources/textures/Peeber.png");

			m_geometryPass = make_reference<gpu::VKRenderPass>(ctx, m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_ubos);

			m_geometryPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...
		}

		m_material = make_reference<gpu::VKMaterial>(ctx, m_geometryShader);
		m_material->set("u_Texture", m_texture);

		m_mesh = make_reference<gpu::VKMesh>(ctx, "../resources/meshes/Orbo.fbx");

		gpu::ImageCreateInfo colour_attachment_image_create_info{};
		colour_attachment_image_create_info.width       = window_width;
		colour_attachment_image_create_info.height      = window_height;
		colour_attachment_image_create_info.format      = swapchain->getSurfaceFormat().format;
		colour_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
		colour_attachment_image_create_info.sampleCount = ctx->getMaxUsableSampleCount();
		m_colourAttachmentImage                         = make_reference<gpu::VKImage2D>(ctx, colour_attachment_image_create_info);

		gpu::ImageCreateInfo depth_attachment_image_create_info{};
		depth_attachment_image_create_info.width       = window_width;
		depth_attachment_image_create_info.height      = window_height;
		depth_attachment_image_create_info.format      = swapchain->getDepthFormat();
		depth_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
		depth_attachment_image_create_info.sampleCount = ctx->getMaxUsableSampleCount();
		m_depthAttachmentImage                         = make_reference<gpu::VKImage2D>(ctx, depth_attachment_image_create_info);
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

		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.clearValue         = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};;
		colour_attachment_info.imageView          = m_colourAttachmentImage->getImageView();
		colour_attachment_info.imageLayout        = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eAverage;
		colour_attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.resolveImageView   = swapchain->getImageView(image_index);

		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue         = vk::ClearDepthStencilValue{1.0f, 0u};;
		depth_attachment_info.imageView          = m_depthAttachmentImage->getImageView();
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

		Renderer::beginRendering(rendering_info, command_buffer, frame_index, m_geometryPass);

		glm::mat4 transform{glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{20.0f, 20.0f, 20.0f}), m_time * glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f})};

		Renderer::renderGeometry(command_buffer, frame_index, m_geometryPipeline, m_mesh->getVertexBuffer(), m_mesh->getIndexBuffer(), m_mesh->getIndices().size(),
								 m_material, transform);
		Renderer::endRendering(command_buffer);
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(ClientLayer::onWindowResizeEvent));
	}

	void ClientLayer::onUIRender()
	{
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
