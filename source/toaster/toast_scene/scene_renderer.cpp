#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "scene_renderer.hpp"

#include "toast_asset/texture_asset.hpp"
#include "toast_render/render_context.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_project/project.hpp"
#include "toast_render/globals.hpp"

namespace toaster
{
	SceneRenderer::SceneRenderer(render::RenderContext *p_render_ctx, const SceneRendererSpecInfo &p_spec_info) : m_renderCtx(p_render_ctx), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_specInfo.scene, "This is called SceneRenderer, please provide a scene!");

		m_cameraUBOs       = m_renderCtx->createUniformBuffers<CameraUB>(render::RenderContext::maxFramesInFlight);
		m_mappedCameraUBOs = m_cameraUBOs->mapAllMemory(sizeof(CameraUB));

		m_directionalLightUBOs       = m_renderCtx->createUniformBuffers<DirectionalLightUB>(render::RenderContext::maxFramesInFlight);
		m_mappedDirectionalLightUBOs = m_directionalLightUBOs->mapAllMemory(sizeof(DirectionalLightUB));

		m_pointLightUBOs       = m_renderCtx->createUniformBuffers<PointLightUB>(render::RenderContext::maxFramesInFlight);
		m_mappedPointLightUBOs = m_pointLightUBOs->mapAllMemory(sizeof(PointLightUB));

		m_sceneDataUBOs       = m_renderCtx->createUniformBuffers<SceneDataUB>(render::RenderContext::maxFramesInFlight);
		m_mappedSceneDataUBOs = m_sceneDataUBOs->mapAllMemory(sizeof(SceneDataUB));

		auto env_asset{m_specInfo.scene->m_project->getAssetManager().getAsset<asset::Texture3DAsset>(m_specInfo.scene->m_sceneEnvironment)};
		if (env_asset)
			m_skyboxMap = env_asset->getTexture();
		else
			m_skyboxMap = m_renderCtx->getGlobals()->whiteTexture3D();

		#pragma region depth-pre
		{
			gpu::PipelineSpecInfo depth_pre_pipeline_spec_info{};
			depth_pre_pipeline_spec_info.vertexBufferLayout = {
				{gpu::EBufferDataType::eFloat3, "a_Position"},
				{gpu::EBufferDataType::eFloat3, "a_Normal"},
				{gpu::EBufferDataType::eFloat3, "a_Tangent"},
				{gpu::EBufferDataType::eFloat3, "a_Bitangent"},
				{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
			};

			depth_pre_pipeline_spec_info.depthFormat = vk::Format::eD32Sfloat;
			depth_pre_pipeline_spec_info.multisample = true;
			depth_pre_pipeline_spec_info.shader      = m_renderCtx->getGlobals()->shaderLibrary().get("Depth-Pre");
			m_depthPrePipeline                       = m_renderCtx->createGPU<gpu::VKPipeline>(depth_pre_pipeline_spec_info);

			m_depthPrePass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_depthPrePipeline);
			m_depthPrePass->setInput("Camera", m_cameraUBOs);
			m_depthPrePass->bake();

			m_depthPreAttachmentImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																					  vk::ImageAspectFlagBits::eDepth);
			m_depthPreResolveAttachmentTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																					  vk::ImageAspectFlagBits::eDepth);

			m_geometryNormalsAttachmentImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																							 vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
			m_geometryNormalsResolveAttachmentTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																							 vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
		}
		#pragma endregion

		#pragma region ambient occlusion
		{
			m_aoPipeline = m_renderCtx->createGPU<gpu::VKComputePipeline>(m_renderCtx->getGlobals()->shaderLibrary().get("Ambient_Occlusion"));

			m_aoPass = m_renderCtx->createGPU<gpu::VKComputePass>(m_aoPipeline);

			gpu::ImageSpecInfo ao_output_image_spec_info{};
			ao_output_image_spec_info.width  = m_specInfo.viewportWidth;
			ao_output_image_spec_info.height = m_specInfo.viewportHeight;
			ao_output_image_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
			ao_output_image_spec_info.usage  = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
			m_aoOutputImage                  = m_renderCtx->createGPU<gpu::VKStorageImage>(ao_output_image_spec_info);

			m_aoPass->setInput("o_Occlusion", m_aoOutputImage);
			m_aoPass->setInput("Camera", m_cameraUBOs);

			m_aoFrameDataMaterial = m_renderCtx->create<render::Material>(m_renderCtx->getGlobals()->shaderLibrary().get("Ambient_Occlusion"), "VBAO");
			m_aoFrameDataMaterial->set(".u_Radius", 2.718f);
			m_aoFrameDataMaterial->set(".u_Thickness", 0.31415f);
			m_aoFrameDataMaterial->set(".u_NumSlices", 4);
			m_aoFrameDataMaterial->set(".u_NumSamplesPerSlice", 5);

			m_aoPass->bake();
		}
		#pragma endregion

		#pragma region light culling
		{
			m_lightCullingPipeline = m_renderCtx->createGPU<gpu::VKComputePipeline>(m_renderCtx->getGlobals()->shaderLibrary().get("Compute-Test"));

			gpu::ImageSpecInfo compute_image_spec_info{};
			compute_image_spec_info.width  = m_specInfo.viewportWidth;
			compute_image_spec_info.height = m_specInfo.viewportHeight;
			compute_image_spec_info.format = vk::Format::eR32G32B32A32Sfloat;
			compute_image_spec_info.usage  = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
			m_computeImage                 = m_renderCtx->createGPU<gpu::VKStorageImage>(compute_image_spec_info);

			m_lightCullingPass = m_renderCtx->createGPU<gpu::VKComputePass>(m_lightCullingPipeline);
			m_lightCullingPass->setInput("u_TestImage", m_computeImage);
			m_lightCullingPass->setInput("u_CubemapImage", m_skyboxMap);
			m_lightCullingPass->bake();

			m_lightCullingMaterial = make_reference<render::Material>(m_renderCtx, m_renderCtx->getGlobals()->shaderLibrary().get("Compute-Test"));
		}
		#pragma endregion

		#pragma region skybox
		{
			gpu::PipelineSpecInfo skybox_pipeline_spec_info{};
			skybox_pipeline_spec_info.vertexBufferLayout = {{gpu::EBufferDataType::eFloat3, "a_Position"}, {gpu::EBufferDataType::eFloat2, "a_TexCoord"}};
			skybox_pipeline_spec_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
			skybox_pipeline_spec_info.shader             = m_renderCtx->getGlobals()->shaderLibrary().get("Skybox");
			skybox_pipeline_spec_info.polygonMode        = vk::PolygonMode::eFill;
			skybox_pipeline_spec_info.multisample        = true;
			skybox_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
			m_skyboxPipeline                             = m_renderCtx->createGPU<gpu::VKPipeline>(skybox_pipeline_spec_info);

			m_skyboxPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_skyboxPipeline);
			m_skyboxPass->setInput("Camera", m_cameraUBOs);
			m_skyboxPass->setInput("u_CubemapImage", m_skyboxMap);

			m_skyboxPass->bake();

			m_skyboxMaterial = make_reference<render::Material>(m_renderCtx, m_renderCtx->getGlobals()->shaderLibrary().get("Skybox"));
		}
		#pragma endregion

		#pragma region geometry
		{
			gpu::PipelineSpecInfo geometry_pipeline_spec_info{};
			geometry_pipeline_spec_info.vertexBufferLayout = {
				{gpu::EBufferDataType::eFloat3, "a_Position"},
				{gpu::EBufferDataType::eFloat3, "a_Normal"},
				{gpu::EBufferDataType::eFloat3, "a_Tangent"},
				{gpu::EBufferDataType::eFloat3, "a_Bitangent"},
				{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
			};
			geometry_pipeline_spec_info.colourAttachments = {vk::Format::eR8G8B8A8Srgb};
			geometry_pipeline_spec_info.depthFormat       = vk::Format::eD32Sfloat;
			geometry_pipeline_spec_info.depthWrite        = false;
			geometry_pipeline_spec_info.multisample       = true;
			geometry_pipeline_spec_info.depthCompare      = vk::CompareOp::eLessOrEqual;
			geometry_pipeline_spec_info.polygonMode       = vk::PolygonMode::eFill;
			geometry_pipeline_spec_info.shader            = m_renderCtx->getGlobals()->shaderLibrary().get("Geometry");
			m_geometryPipeline                            = m_renderCtx->createGPU<gpu::VKPipeline>(geometry_pipeline_spec_info);

			m_geometryPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_cameraUBOs);
			m_geometryPass->setInput("DirectionalLightData", m_directionalLightUBOs);
			m_geometryPass->setInput("PointLightData", m_pointLightUBOs);
			m_geometryPass->setInput("SceneData", m_sceneDataUBOs);
			m_geometryPass->setInput("u_EnvironmentMap", m_skyboxMap);
			m_geometryPass->setInput("u_AOTexture", m_aoOutputImage);

			m_geometryPass->bake();
		}
		#pragma endregion

		m_colourImage          = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportWidth, m_specInfo.viewportHeight, vk::ImageAspectFlagBits::eColor);
		m_resolveColourTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight, vk::ImageAspectFlagBits::eColor);

		render::Renderer2DSpecInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth   = m_specInfo.viewportWidth;
		renderer_2d_create_info.renderTargetHeight  = m_specInfo.viewportHeight;
		renderer_2d_create_info.overrideAttachments = true;
		renderer_2d_create_info.msaa                = true;
		m_renderer2D                                = make_reference<render::Renderer2D>(m_renderCtx, renderer_2d_create_info);
	}

	SceneRenderer::~SceneRenderer()
	{
		m_sceneDataUBOs->unmapAllMemory();
		m_directionalLightUBOs->unmapAllMemory();
		m_cameraUBOs->unmapAllMemory();
	}

	auto SceneRenderer::begin(const glm::mat4 &p_view_matrix, const glm::mat4 &p_projection_matrix) -> void
	{
		uint32 frame_index{m_renderCtx->getCurrentFrameIndex()};

		CameraUB camera_ub{};
		camera_ub.view       = p_view_matrix;
		camera_ub.proj       = p_projection_matrix;
		camera_ub.proj[1][1] *= -1.0f; // Silly opengl
		camera_ub.invProj    = p_projection_matrix;
		std::memcpy(m_mappedCameraUBOs[frame_index], &camera_ub, sizeof(CameraUB));

		const auto &[directional_lights, point_lights]{m_specInfo.scene->getLightEnvironment()};
		{
			DirectionalLightUB directional_light_ub{};
			directional_light_ub.count = directional_lights.size();
			for (uint32 i{0u}; i < DirectionalLightUB::c_maxDirectionalLights && i < directional_lights.size(); ++i)
			{
				directional_light_ub.directionalLights[i].direction = directional_lights[i].direction;
				directional_light_ub.directionalLights[i].radiance  = directional_lights[i].radiance;
			}
			std::memcpy(m_mappedDirectionalLightUBOs[frame_index], &directional_light_ub, sizeof(DirectionalLightUB));
		}
		{
			PointLightUB point_light_ub{};
			point_light_ub.count = std::min(static_cast<uint32>(point_lights.size()), PointLightUB::c_maxPointLights);
			for (uint32 i{0u}; i < PointLightUB::c_maxPointLights && i < point_lights.size(); ++i)
			{
				point_light_ub.pointLights[i].position = point_lights[i].position;
				point_light_ub.pointLights[i].radiance = point_lights[i].radiance;
			}
			std::memcpy(m_mappedPointLightUBOs[frame_index], &point_light_ub, sizeof(PointLightUB));
		}

		SceneDataUB scene_data_ub{};
		scene_data_ub.cameraPos = glm::inverse(p_view_matrix)[3];
		std::memcpy(m_mappedSceneDataUBOs[frame_index], &scene_data_ub, sizeof(SceneDataUB));
	}

	auto SceneRenderer::end(gpu::VKCommandBuffer *p_cmd) -> void
	{
		if (m_reloadSkybox)
		{
			m_skyboxPass->setInput("u_CubemapImage", m_skyboxMap);
			m_geometryPass->setInput("u_EnvironmentMap", m_skyboxMap);
			m_reloadSkybox = false;
		}

		_renderDepthPrePass(p_cmd);
		_renderAOPass(p_cmd);
		_renderLightCullingPass(p_cmd);
		_renderSkyboxPass(p_cmd);
		_renderGeometryPass(p_cmd);

		m_meshDrawCommands.clear();
	}

	auto SceneRenderer::renderMesh(const render::MeshHandle &p_mesh, const glm::mat4 &p_transform) -> void
	{
		DrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh      = p_mesh;
		draw_command.transform = p_transform;
	}

	auto SceneRenderer::getSpecInfo() const -> const SceneRendererSpecInfo &
	{
		return m_specInfo;
	}

	auto SceneRenderer::getMSAAOutputColourImage() -> gpu::RawImageHandle &
	{
		return m_colourImage;
	}

	auto SceneRenderer::getMSAAOutputDepthImage() -> gpu::RawImageHandle &
	{
		return m_depthPreAttachmentImage;
	}

	auto SceneRenderer::getMSAAOutputGeometryNormalsImage() -> gpu::RawImageHandle &
	{
		return m_geometryNormalsAttachmentImage;
	}

	auto SceneRenderer::getResolveOutputColourTexture() const -> const gpu::Texture2DHandle &
	{
		return m_resolveColourTexture;
	}

	auto SceneRenderer::getResolveOutputDepthTexture() const -> const gpu::Texture2DHandle &
	{
		return m_depthPreResolveAttachmentTexture;
	}

	auto SceneRenderer::getResolveOutputGeometryNormalsTexture() const -> const gpu::Texture2DHandle &
	{
		return m_geometryNormalsResolveAttachmentTexture;
	}

	auto SceneRenderer::getOutputAOImage() const -> const gpu::StorageImageHandle &
	{
		return m_aoOutputImage;
	}

	auto SceneRenderer::getOutputComputeImage() const -> const gpu::StorageImageHandle &
	{
		return m_computeImage;
	}

	auto SceneRenderer::getRenderer2D() -> RefPtr<render::Renderer2D>
	{
		return m_renderer2D;
	}

	auto SceneRenderer::onResize(uint32 p_width, uint32 p_height) -> void
	{
		TST_ASSERT_MSG(p_width != 0 && p_height != 0, "Cannot resize to 0");

		if (m_specInfo.viewportWidth != p_width || m_specInfo.viewportHeight != p_height)
		{
			m_specInfo.viewportWidth  = p_width;
			m_specInfo.viewportHeight = p_height;

			m_depthPreAttachmentImage->resize(p_width, p_height);
			m_depthPreResolveAttachmentTexture->resize(p_width, p_height);

			m_geometryNormalsAttachmentImage->resize(p_width, p_height);
			m_geometryNormalsResolveAttachmentTexture->resize(p_width, p_height);

			m_aoOutputImage->resize(p_width, p_height);

			m_computeImage->resize(p_width, p_height);

			m_colourImage->resize(p_width, p_height);
			m_resolveColourTexture->resize(p_width, p_height);

			m_renderer2D->onResize(p_width, p_height);
		}
	}

	auto SceneRenderer::setEnvironmentBackground(const gpu::Texture3DHandle &p_texture) -> void
	{
		m_skyboxMap    = p_texture;
		m_reloadSkybox = true;
	}

	auto SceneRenderer::_renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue   = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.image        = m_depthPreAttachmentImage;
		depth_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		depth_attachment_info.resolveImage = m_depthPreResolveAttachmentTexture->getImage();
		depth_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eMin;
		rendering_info.pDepthAttachment    = &depth_attachment_info;

		gpu::RenderingAttachmentInfo &geo_normals_attachment_info{rendering_info.colourAttachments.emplace_back()};
		geo_normals_attachment_info.clearValue   = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
		geo_normals_attachment_info.image        = m_geometryNormalsAttachmentImage;
		geo_normals_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		geo_normals_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		geo_normals_attachment_info.resolveImage = m_geometryNormalsResolveAttachmentTexture->getImage();
		geo_normals_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_depthPrePass);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				m_renderCtx->renderMesh(p_cmd, draw_cmd.mesh, i, m_depthPrePipeline, draw_cmd.transform * draw_cmd.mesh->getSubmeshes()[i].localTransform, nullptr);
			}
		}
		m_renderCtx->endRendering(p_cmd, rendering_info);
	}

	auto SceneRenderer::_renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		// Used for random noise generation in the VBAO shader
		static uint32 s_frameIndex{0u};
		m_aoFrameDataMaterial->set(".u_FrameIndex", s_frameIndex);

		m_aoPass->setInput("u_DepthTex", m_depthPreResolveAttachmentTexture);
		m_aoPass->setInput("u_NormalsTex", m_geometryNormalsResolveAttachmentTexture);

		const uint32     work_groups_x{(m_specInfo.viewportWidth + 15u) / 16u};
		const uint32     work_groups_y{(m_specInfo.viewportHeight + 15u) / 16u};
		constexpr uint32 work_groups_z{1u};

		m_renderCtx->beginCompute(p_cmd, m_aoPass);
		m_renderCtx->dispatchCompute(p_cmd, m_aoPass, m_aoFrameDataMaterial, work_groups_x, work_groups_y, work_groups_z);
	}

	auto SceneRenderer::_renderLightCullingPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		m_renderCtx->beginCompute(p_cmd, m_lightCullingPass);
		m_renderCtx->dispatchCompute(p_cmd, m_lightCullingPass, m_lightCullingMaterial, m_specInfo.viewportWidth, m_specInfo.viewportHeight, 1);
	}

	auto SceneRenderer::_renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.clearValue   = vk::ClearColorValue{1.0f, 0.0f, 0.0f, 1.0f};
		colour_attachment_info.image        = m_colourImage;
		colour_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.resolveImage = m_resolveColourTexture->getImage();
		colour_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_skyboxPass);
		m_renderCtx->renderFullscreenQuad(p_cmd, m_skyboxPipeline, m_skyboxMaterial);
		m_renderCtx->endRendering(p_cmd, rendering_info);
	}

	auto SceneRenderer::_renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea    = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount    = 1;
		rendering_info.depthReadOnly = true;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.image        = m_colourImage;
		colour_attachment_info.loadOp       = vk::AttachmentLoadOp::eLoad;
		colour_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.resolveImage = m_resolveColourTexture->getImage();
		colour_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue   = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.image        = m_depthPreAttachmentImage;
		depth_attachment_info.loadOp       = vk::AttachmentLoadOp::eLoad;
		depth_attachment_info.storeOp      = vk::AttachmentStoreOp::eDontCare;
		depth_attachment_info.resolveImage = m_depthPreResolveAttachmentTexture->getImage();
		rendering_info.pDepthAttachment    = &depth_attachment_info;

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_geometryPass);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				m_renderCtx->renderMesh(p_cmd, draw_cmd.mesh, i, m_geometryPipeline, draw_cmd.transform * draw_cmd.mesh->getSubmeshes()[i].localTransform);
			}
		}
		m_renderCtx->endRendering(p_cmd, rendering_info);
	}
}
