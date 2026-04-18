#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"

namespace toaster
{
	SceneRenderer::SceneRenderer(gpu::VKGPUContext *p_ctx, const SceneRendererSpecInfo &p_spec_info) : m_ctx(p_ctx), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_specInfo.scene, "This is called SceneRenderer, please provide a scene!");
		{
			constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
			m_cameraUBOs       = m_ctx->alloc<gpu::VKUniformBufferPFF>(ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedCameraUBOs = m_cameraUBOs->mapMemory(ubo_size, 0);
		}
		{
			constexpr vk::DeviceSize ubo_size{sizeof(PointLightUB)};
			m_pointLightUBOs       = m_ctx->alloc<gpu::VKUniformBufferPFF>(ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedPointLightUBOs = m_pointLightUBOs->mapMemory(ubo_size, 0);
		}
		{
			constexpr vk::DeviceSize ubo_size{sizeof(SceneDataUB)};
			m_sceneDataUBOs       = m_ctx->alloc<gpu::VKUniformBufferPFF>(ubo_size, gpu::VKGPUContext::c_maxFramesInFlight);
			m_mappedSceneDataUBOs = m_sceneDataUBOs->mapMemory(ubo_size, 0);
		}

		gpu::TextureSpecInfo texture_spec_info{};
		m_skyboxTexture = m_ctx->alloc<gpu::VKTexture2D>(texture_spec_info, "../resources/environments/'Environment_map'.jpg");

		{
			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {{gpu::EBufferDataType::eFloat3, "a_Position"}, {gpu::EBufferDataType::eFloat2, "a_TexCoord"}};
			pipeline_create_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
			pipeline_create_info.shader             = Globals::getShaderLibrary().get("Skybox");
			pipeline_create_info.polygonMode        = vk::PolygonMode::eFill;
			pipeline_create_info.multisample        = true;
			m_skyboxPipeline                        = m_ctx->alloc<gpu::VKPipeline>(pipeline_create_info);

			m_skyboxPass = m_ctx->alloc<gpu::VKRenderPass>(m_skyboxPipeline);
			m_skyboxPass->setInput("Camera", m_cameraUBOs);

			m_skyboxPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...

			m_skyboxMaterial = m_ctx->alloc<gpu::VKMaterial>(Globals::getShaderLibrary().get("Skybox"));
		}
		{
			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {
				{gpu::EBufferDataType::eFloat3, "a_Position"},
				{gpu::EBufferDataType::eFloat3, "a_Normal"},
				{gpu::EBufferDataType::eFloat3, "a_Tangent"},
				{gpu::EBufferDataType::eFloat3, "a_Bitangent"},
				{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
			};
			pipeline_create_info.colourAttachments = {
				vk::Format::eR8G8B8A8Srgb,
				vk::Format::eR16G16B16A16Sfloat /*Positions*/,
				vk::Format::eR16G16B16A16Sfloat /*Normals*/
			};
			pipeline_create_info.depthFormat = {m_ctx->findDepthFormat()};
			pipeline_create_info.shader      = Globals::getShaderLibrary().get("Geometry");
			pipeline_create_info.multisample = true;
			pipeline_create_info.polygonMode = vk::PolygonMode::eFill;
			m_geometryPipeline               = m_ctx->alloc<gpu::VKPipeline>(pipeline_create_info);

			m_geometryPass = m_ctx->alloc<gpu::VKRenderPass>(m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_cameraUBOs);
			m_geometryPass->setInput("PointLightData", m_pointLightUBOs);
			m_geometryPass->setInput("SceneData", m_sceneDataUBOs);

			m_geometryPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...
		}

		{
			gpu::ImageCreateInfo msaa_positions_attachment_image_create_info{};
			msaa_positions_attachment_image_create_info.width       = m_specInfo.viewportWidth;
			msaa_positions_attachment_image_create_info.height      = m_specInfo.viewportHeight;
			msaa_positions_attachment_image_create_info.format      = vk::Format::eR16G16B16A16Sfloat;
			msaa_positions_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
			msaa_positions_attachment_image_create_info.sampleCount = m_ctx->getMaxUsableSampleCount();
			m_MSAAGeometryPositionsAttachmentImage                  = m_ctx->alloc<gpu::VKImage2D>(msaa_positions_attachment_image_create_info);

			gpu::TextureSpecInfo geometry_positions_attachment_texture_spec_info{};
			geometry_positions_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			geometry_positions_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			geometry_positions_attachment_texture_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
			m_resolveGeometryPositionsAttachmentTexture            = m_ctx->alloc<gpu::VKTexture2D>(geometry_positions_attachment_texture_spec_info);
		}

		{
			gpu::ImageCreateInfo msaa_normals_attachment_image_create_info{};
			msaa_normals_attachment_image_create_info.width       = m_specInfo.viewportWidth;
			msaa_normals_attachment_image_create_info.height      = m_specInfo.viewportHeight;
			msaa_normals_attachment_image_create_info.format      = vk::Format::eR16G16B16A16Sfloat;
			msaa_normals_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
			msaa_normals_attachment_image_create_info.sampleCount = m_ctx->getMaxUsableSampleCount();
			m_MSAAGeometryNormalsAttachmentImage                  = m_ctx->alloc<gpu::VKImage2D>(msaa_normals_attachment_image_create_info);

			gpu::TextureSpecInfo geometry_normals_attachment_texture_spec_info{};
			geometry_normals_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			geometry_normals_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			geometry_normals_attachment_texture_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
			m_resolveGeometryNormalsAttachmentTexture            = m_ctx->alloc<gpu::VKTexture2D>(geometry_normals_attachment_texture_spec_info);
		}
		{
			gpu::ImageCreateInfo msaa_colour_attachment_image_create_info{};
			msaa_colour_attachment_image_create_info.width       = m_specInfo.viewportWidth;
			msaa_colour_attachment_image_create_info.height      = m_specInfo.viewportHeight;
			msaa_colour_attachment_image_create_info.format      = vk::Format::eR8G8B8A8Srgb;
			msaa_colour_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment;
			msaa_colour_attachment_image_create_info.sampleCount = m_ctx->getMaxUsableSampleCount();
			m_MSAAColourAttachmentImage                          = m_ctx->alloc<gpu::VKImage2D>(msaa_colour_attachment_image_create_info);

			gpu::ImageCreateInfo msaa_depth_attachment_image_create_info{};
			msaa_depth_attachment_image_create_info.width       = m_specInfo.viewportWidth;
			msaa_depth_attachment_image_create_info.height      = m_specInfo.viewportHeight;
			msaa_depth_attachment_image_create_info.format      = m_ctx->findDepthFormat();
			msaa_depth_attachment_image_create_info.usage       = vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment;
			msaa_depth_attachment_image_create_info.sampleCount = m_ctx->getMaxUsableSampleCount();
			m_MSAADepthAttachmentImage                          = m_ctx->alloc<gpu::VKImage2D>(msaa_depth_attachment_image_create_info);
		}
		{
			gpu::TextureSpecInfo resolve_colour_attachment_texture_spec_info{};
			resolve_colour_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			resolve_colour_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			resolve_colour_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
			m_resolveOutputColourTexture                       = m_ctx->alloc<gpu::VKTexture2D>(resolve_colour_attachment_texture_spec_info);

			gpu::ImageCreateInfo resolve_depth_attachment_image_create_info{};
			resolve_depth_attachment_image_create_info.width  = m_specInfo.viewportWidth;
			resolve_depth_attachment_image_create_info.height = m_specInfo.viewportHeight;
			resolve_depth_attachment_image_create_info.format = m_ctx->findDepthFormat();
			resolve_depth_attachment_image_create_info.usage  = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			m_resolveOutputDepthImage                         = m_ctx->alloc<gpu::VKImage2D>(resolve_depth_attachment_image_create_info);
		}
		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth   = m_specInfo.viewportWidth;
		renderer_2d_create_info.renderTargetHeight  = m_specInfo.viewportHeight;
		renderer_2d_create_info.overrideAttachments = true;
		m_renderer2D                                = make_reference<Renderer2D>(m_ctx, renderer_2d_create_info);
	}

	SceneRenderer::~SceneRenderer()
	{
		m_sceneDataUBOs->unmapMemory();
		m_pointLightUBOs->unmapMemory();
		m_cameraUBOs->unmapMemory();
	}

	auto SceneRenderer::begin([[maybe_unused]] const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const glm::mat4 &p_view_matrix,
							  const glm::mat4 &                               p_projection_matrix) -> void
	{
		CameraUB camera_ub{};
		camera_ub.view       = p_view_matrix;
		camera_ub.proj       = p_projection_matrix;
		camera_ub.proj[1][1] *= -1.0f;
		std::memcpy(m_mappedCameraUBOs[p_frame_index], &camera_ub, sizeof(CameraUB));

		const SceneLightEnvironment &light_environment{m_specInfo.scene->getLightEnvironment()};

		PointLightUB point_light_ub{};
		point_light_ub.count = light_environment.pointLights.size();
		for (uint32 i{0u}; i < PointLightUB::c_maxPointLights && i < light_environment.pointLights.size(); ++i)
		{
			point_light_ub.pointLights[i].position   = light_environment.pointLights[i].position;
			point_light_ub.pointLights[i].radiance   = light_environment.pointLights[i].radiance;
			point_light_ub.pointLights[i].radius     = light_environment.pointLights[i].radius;
			point_light_ub.pointLights[i].falloff    = light_environment.pointLights[i].falloff;
			point_light_ub.pointLights[i].multiplier = light_environment.pointLights[i].multiplier;
		}
		std::memcpy(m_mappedPointLightUBOs[p_frame_index], &point_light_ub, sizeof(PointLightUB));

		SceneDataUB scene_data_ub{};
		scene_data_ub.cameraPos = glm::inverse(p_view_matrix)[3];
		std::memcpy(m_mappedSceneDataUBOs[p_frame_index], &scene_data_ub, sizeof(SceneDataUB));
	}

	auto SceneRenderer::end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		_renderSkyboxPass(p_cmd, p_frame_index);
		_renderGeometryPass(p_cmd, p_frame_index);

		m_meshDrawCommands.clear();
	}

	auto SceneRenderer::renderMesh(RefPtr<gpu::VKMesh> p_mesh, const glm::mat4 &p_transform) -> void
	{
		DrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh      = p_mesh;
		draw_command.transform = p_transform;
	}

	auto SceneRenderer::getSpecInfo() const -> const SceneRendererSpecInfo &
	{
		return m_specInfo;
	}

	auto SceneRenderer::getOutputColourTexture() const -> const RefPtr<gpu::VKTexture2D> &
	{
		return m_resolveGeometryNormalsAttachmentTexture;
	}

	auto SceneRenderer::getOutputDepthImage() const -> const RefPtr<gpu::VKImage2D> &
	{
		return m_resolveOutputDepthImage;
	}

	auto SceneRenderer::getRenderer2D() -> RefPtr<Renderer2D>
	{
		return m_renderer2D;
	}

	auto SceneRenderer::onResize(uint32 p_width, uint32 p_height) -> void
	{
		m_specInfo.viewportWidth  = p_width;
		m_specInfo.viewportHeight = p_height;

		m_MSAAGeometryPositionsAttachmentImage->resize(p_width, p_height);
		m_resolveGeometryPositionsAttachmentTexture->resize(p_width, p_height);

		m_MSAAGeometryNormalsAttachmentImage->resize(p_width, p_height);
		m_resolveGeometryNormalsAttachmentTexture->resize(p_width, p_height);

		m_MSAAColourAttachmentImage->resize(p_width, p_height);
		m_MSAADepthAttachmentImage->resize(p_width, p_height);
		m_resolveOutputColourTexture->resize(p_width, p_height);
		m_resolveOutputDepthImage->resize(p_width, p_height);
		m_renderer2D->onResize(p_width, p_height);
	}

	auto SceneRenderer::setEnvironmentBackground(RefPtr<gpu::VKTexture2D> p_texture) -> void
	{
		m_skyboxTexture = p_texture;
	}

	auto SceneRenderer::_renderSkyboxPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		m_skyboxMaterial->set("u_Texture", m_skyboxTexture);

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.clearValue   = vk::ClearColorValue{1.0f, 0.0f, 0.0f, 1.0f};
		colour_attachment_info.image        = m_MSAAColourAttachmentImage;
		colour_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.resolveImage = m_resolveOutputColourTexture->getImage();
		colour_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

		Renderer::beginRendering(rendering_info, p_cmd, p_frame_index, m_skyboxPass);
		Renderer::renderFullscreenQuad(p_cmd, p_frame_index, m_skyboxPipeline, m_skyboxMaterial);
		Renderer::endRendering(rendering_info, p_cmd);
	}

	auto SceneRenderer::_renderGeometryPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.clearValue   = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
		colour_attachment_info.image        = m_MSAAColourAttachmentImage;
		colour_attachment_info.loadOp       = vk::AttachmentLoadOp::eNone;
		colour_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.resolveImage = m_resolveOutputColourTexture->getImage();
		colour_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

		{
			gpu::RenderingAttachmentInfo &positions_attachment_info{rendering_info.colourAttachments.emplace_back()};
			positions_attachment_info.clearValue   = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
			positions_attachment_info.image        = m_MSAAGeometryPositionsAttachmentImage;
			positions_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
			positions_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
			positions_attachment_info.resolveImage = m_resolveGeometryPositionsAttachmentTexture->getImage();
			positions_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;
		}
		{
			gpu::RenderingAttachmentInfo &normals_attachment_info{rendering_info.colourAttachments.emplace_back()};
			normals_attachment_info.clearValue   = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
			normals_attachment_info.image        = m_MSAAGeometryNormalsAttachmentImage;
			normals_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
			normals_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
			normals_attachment_info.resolveImage = m_resolveGeometryNormalsAttachmentTexture->getImage();
			normals_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;
		}
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
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				Renderer::renderMesh(p_cmd, p_frame_index, draw_cmd.mesh, i, m_geometryPipeline, draw_cmd.transform * draw_cmd.mesh->getSubmeshes()[i].localTransform);
			}
		}

		Renderer::endRendering(rendering_info, p_cmd);
	}
}
