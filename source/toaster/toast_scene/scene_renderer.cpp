#include "scene_renderer.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer.hpp"

namespace toaster
{
	SceneRenderer::SceneRenderer(gpu::VKGPUContext *p_ctx, const SceneRendererSpecInfo &p_spec_info) : m_ctx(p_ctx), m_specInfo(p_spec_info)
	{
		{
			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {
				{gpu::EShaderDataType::eFloat3, "a_Position"},
				{gpu::EShaderDataType::eFloat3, "a_Normal"},
				{gpu::EShaderDataType::eFloat3, "a_Tangent"},
				{gpu::EShaderDataType::eFloat3, "a_Bitangent"},
				{gpu::EShaderDataType::eFloat2, "a_TexCoord"}
			};
			pipeline_create_info.colourAttachments = {vk::Format::eR8G8B8A8Srgb};
			pipeline_create_info.depthFormat       = {m_ctx->findDepthFormat()};
			pipeline_create_info.shader            = Globals::getShaderLibrary().get("Geometry");
			pipeline_create_info.multisample       = true;
			m_geometryPipeline                     = make_reference<gpu::VKPipeline>(m_ctx, pipeline_create_info);

			constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
			m_cameraUBOs       = make_reference<gpu::VKUniformBufferPFF>(m_ctx, ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedCameraUBOs = m_cameraUBOs->mapMemory(ubo_size, 0);

			m_geometryPass = make_reference<gpu::VKRenderPass>(m_ctx, m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_cameraUBOs);

			m_geometryPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...

			gpu::ImageCreateInfo msaa_colour_attachment_image_create_info{};
			msaa_colour_attachment_image_create_info.width       = m_specInfo.viewportWidth;
			msaa_colour_attachment_image_create_info.height      = m_specInfo.viewportHeight;
			msaa_colour_attachment_image_create_info.format      = vk::Format::eR8G8B8A8Srgb;
			msaa_colour_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
			msaa_colour_attachment_image_create_info.sampleCount = m_ctx->getMaxUsableSampleCount();
			m_MSAAColourAttachmentImage                          = make_reference<gpu::VKImage2D>(m_ctx, msaa_colour_attachment_image_create_info);

			gpu::ImageCreateInfo msaa_depth_attachment_image_create_info{};
			msaa_depth_attachment_image_create_info.width       = m_specInfo.viewportWidth;
			msaa_depth_attachment_image_create_info.height      = m_specInfo.viewportHeight;
			msaa_depth_attachment_image_create_info.format      = m_ctx->findDepthFormat();
			msaa_depth_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
			msaa_depth_attachment_image_create_info.sampleCount = m_ctx->getMaxUsableSampleCount();
			m_MSAADepthAttachmentImage                          = make_reference<gpu::VKImage2D>(m_ctx, msaa_depth_attachment_image_create_info);

			gpu::TextureSpecInfo resolve_colour_attachment_texture_spec_info{};
			resolve_colour_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			resolve_colour_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			resolve_colour_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
			m_resolveOutputColourTexture                       = make_reference<gpu::VKTexture2D>(m_ctx, resolve_colour_attachment_texture_spec_info);

			gpu::ImageCreateInfo resolve_depth_attachment_image_create_info{};
			resolve_depth_attachment_image_create_info.width  = m_specInfo.viewportWidth;
			resolve_depth_attachment_image_create_info.height = m_specInfo.viewportHeight;
			resolve_depth_attachment_image_create_info.format = m_ctx->findDepthFormat();
			resolve_depth_attachment_image_create_info.usage  = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			m_resolveOutputDepthImage                         = make_reference<gpu::VKImage2D>(m_ctx, resolve_depth_attachment_image_create_info);
		}
	}

	SceneRenderer::~SceneRenderer()
	{
		m_cameraUBOs->unmapMemory();
	}

	void SceneRenderer::begin(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix)
	{
		CameraUB camera_ub{};
		camera_ub.view = p_view_matrix;
		camera_ub.proj = p_projection_matrix;

		std::memcpy(m_mappedCameraUBOs[p_frame_index], &camera_ub, sizeof(CameraUB));
	}

	void SceneRenderer::end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index)
	{
		_renderGeometryPass(p_cmd, p_frame_index);

		m_meshDrawCommands.clear();
	}

	void SceneRenderer::renderMesh(const RefPtr<gpu::VKMesh> &p_mesh, const glm::mat4 &p_transform)
	{
		DrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh      = p_mesh;
		draw_command.transform = p_transform;
	}

	void SceneRenderer::onResize(uint32 p_width, uint32 p_height)
	{
		m_specInfo.viewportWidth  = p_width;
		m_specInfo.viewportHeight = p_height;

		m_MSAAColourAttachmentImage->resize(p_width, p_height);
		m_MSAADepthAttachmentImage->resize(p_width, p_height);
		m_resolveOutputColourTexture->resize(p_width, p_height);
		m_resolveOutputDepthImage->resize(p_width, p_height);
	}

	const SceneRendererSpecInfo &SceneRenderer::getSpecInfo() const
	{
		return m_specInfo;
	}

	const RefPtr<gpu::VKTexture2D> &SceneRenderer::getOutputColourTexture() const
	{
		return m_resolveOutputColourTexture;
	}

	const RefPtr<gpu::VKImage2D> &SceneRenderer::getOutputDepthImage() const
	{
		return m_resolveOutputDepthImage;
	}

	void SceneRenderer::_renderGeometryPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index)
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.clearValue   = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};
		colour_attachment_info.image        = m_MSAAColourAttachmentImage;
		colour_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.resolveImage = m_resolveOutputColourTexture->getImage();
		colour_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue   = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.image        = m_MSAADepthAttachmentImage;
		depth_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		depth_attachment_info.resolveImage = m_resolveOutputDepthImage;
		depth_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eMin;
		rendering_info.pDepthAttachment    = &depth_attachment_info;

		Renderer::beginRendering(rendering_info, p_cmd, p_frame_index, m_geometryPass);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			Renderer::renderGeometry(p_cmd, p_frame_index, m_geometryPipeline, draw_cmd.mesh->getVertexBuffer(), draw_cmd.mesh->getIndexBuffer(),
									 draw_cmd.mesh->getIndices().size(), draw_cmd.mesh->getMaterial(), draw_cmd.transform);
		}

		Renderer::endRendering(rendering_info, p_cmd);
	}
}
