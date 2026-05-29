#include "toast_scene/scene_renderer.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

#include "toast_lib/math/colours.hpp"

#include <random>

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

			depth_pre_pipeline_spec_info.depthFormat       = vk::Format::eD32Sfloat;
			depth_pre_pipeline_spec_info.colourAttachments = {vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat};
			depth_pre_pipeline_spec_info.multisample       = true;
			depth_pre_pipeline_spec_info.cullMode          = vk::CullModeFlagBits::eBack;
			depth_pre_pipeline_spec_info.shader            = m_renderCtx->getGlobals()->shaderLibrary().get("Depth-Pre");
			m_depthPrePipeline                             = m_renderCtx->createGPU<gpu::VKPipeline>(depth_pre_pipeline_spec_info);

			m_depthPrePass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_depthPrePipeline);
			m_depthPrePass->setInput("Camera", m_cameraUBOs);
			m_depthPrePass->bake();

			m_depthPreAttachmentImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																					  vk::ImageAspectFlagBits::eDepth);
			m_depthPreResolveAttachmentTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																					  vk::ImageAspectFlagBits::eDepth);

			{
				m_geometryNormalsAttachmentImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																								 vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
				m_geometryNormalsResolveAttachmentTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																								 vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
			}
			{
				m_geometryPositionsAttachmentImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																								   vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
				m_geometryPositionsResolveAttachmentTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight,
																								   vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
			}
		}
		#pragma endregion

		#pragma region ambient occlusion
		{
			gpu::PipelineSpecInfo ssao_pipeline_spec_info{};
			ssao_pipeline_spec_info.vertexBufferLayout = {{gpu::EBufferDataType::eFloat3, "a_Position"}, {gpu::EBufferDataType::eFloat2, "a_TexCoord"}};
			ssao_pipeline_spec_info.colourAttachments  = {vk::Format::eR16G16B16A16Sfloat};
			ssao_pipeline_spec_info.shader             = m_renderCtx->getGlobals()->shaderLibrary().get("SSAO_Graphics");
			ssao_pipeline_spec_info.polygonMode        = vk::PolygonMode::eFill;
			ssao_pipeline_spec_info.multisample        = false;
			ssao_pipeline_spec_info.depthTest          = false;
			ssao_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
			m_ssaoPipeline                             = m_renderCtx->createGPU<gpu::VKPipeline>(ssao_pipeline_spec_info);

			m_ssaoPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_ssaoPipeline);

			m_ssaoOutputTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportWidth, m_specInfo.viewportHeight, vk::ImageAspectFlagBits::eColor,
																	   vk::Format::eR16G16B16A16Sfloat);
			m_ssaoPass->setInput("Camera", m_cameraUBOs);

			m_ssaoKernel = make_unique<SSAOKernel>(); // Generates the random rotation vectors
			auto ssao_kernel_ubo{m_renderCtx->createGPU<gpu::VKUniformBuffer>(sizeof(SSAOKernel))};
			ssao_kernel_ubo->setData(m_ssaoKernel->samples, sizeof(SSAOKernel), 0);
			m_ssaoPass->setInput("SSAOKernel", ssao_kernel_ubo);

			constexpr uint32 s_noise_texture_side_size{4u};

			gpu::TextureSpecInfo noise_texture_spec_info{};
			noise_texture_spec_info.width              = s_noise_texture_side_size;
			noise_texture_spec_info.height             = s_noise_texture_side_size;
			noise_texture_spec_info.format             = vk::Format::eR32G32B32A32Sfloat;
			noise_texture_spec_info.generateMips       = false;
			noise_texture_spec_info.samplerFilter      = vk::Filter::eNearest;
			noise_texture_spec_info.samplerAddressMode = vk::SamplerAddressMode::eRepeat;

			static std::uniform_real_distribution<float32> randomFloats(0.0f, 1.0f);
			static std::random_device                      randomDevice{};
			static std::mt19937                            generator{randomDevice()};
			std::vector<glm::vec4>                         ssaoNoise;

			for (uint32_t i = 0; i < s_noise_texture_side_size * s_noise_texture_side_size; i++)
			{
				float32 x{randomFloats(generator) * 2.0f - 1.0f};
				float32 y{randomFloats(generator) * 2.0f - 1.0f};
				float32 z{0.0f};

				auto &noise{ssaoNoise.emplace_back()};
				noise = {x, y, z, 1.0f};
			}
			m_ssaoNoiseTexture = m_renderCtx->createGPU<gpu::VKTexture2D>(noise_texture_spec_info, ssaoNoise.data(), ssaoNoise.size() * sizeof(glm::vec4));
			m_ssaoPass->setInput("u_NoiseTex", m_ssaoNoiseTexture);

			m_aoFrameDataMaterial = m_renderCtx->create<render::Material>(m_renderCtx->getGlobals()->shaderLibrary().get("SSAO_Graphics"), "SSAO");
			m_aoFrameDataMaterial->set(".u_Radius", 0.5f);
			m_aoFrameDataMaterial->set(".u_Bias", 0.025f);

			m_ssaoPass->bake();

			{
				gpu::ImageSpecInfo blurred_ao_image_spec_info{};
				blurred_ao_image_spec_info.width  = m_specInfo.viewportWidth;
				blurred_ao_image_spec_info.height = m_specInfo.viewportHeight;
				blurred_ao_image_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
				blurred_ao_image_spec_info.usage  = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
				m_aoBlurredOutputImage            = m_renderCtx->createGPU<gpu::VKStorageImage>(blurred_ao_image_spec_info);

				m_aoBlurPipeline = m_renderCtx->createGPU<gpu::VKComputePipeline>(m_renderCtx->getGlobals()->shaderLibrary().get("SSAO_Blur"));
				m_aoBlurPass     = m_renderCtx->createGPU<gpu::VKComputePass>(m_aoBlurPipeline);

				m_aoBlurPass->setInput("u_Occlusion", m_ssaoOutputTexture);
				m_aoBlurPass->setInput("o_BlurredOcclusion", m_aoBlurredOutputImage);
				m_aoBlurPass->bake();
			}
		}
		#pragma endregion

		#pragma region light culling
		{

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
			// m_skyboxPass->setInput("u_CubemapImage", m_specInfo.scene->m_sceneEnvironment.skyboxMap);

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
			geometry_pipeline_spec_info.depthWrite        = false; // The depth buffer is gathered from the depth pre-pass
			geometry_pipeline_spec_info.depthTest         = true;
			geometry_pipeline_spec_info.depthCompare      = vk::CompareOp::eLessOrEqual;
			geometry_pipeline_spec_info.multisample       = true;
			geometry_pipeline_spec_info.polygonMode       = vk::PolygonMode::eFill;
			geometry_pipeline_spec_info.cullMode          = vk::CullModeFlagBits::eBack;
			geometry_pipeline_spec_info.shader            = m_renderCtx->getGlobals()->shaderLibrary().get("Geometry");
			m_geometryPipeline                            = m_renderCtx->createGPU<gpu::VKPipeline>(geometry_pipeline_spec_info);

			m_geometryPass = m_renderCtx->createGPU<gpu::VKRenderPass>(m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_cameraUBOs);
			m_geometryPass->setInput("DirectionalLightData", m_directionalLightUBOs);
			m_geometryPass->setInput("PointLightData", m_pointLightUBOs);
			m_geometryPass->setInput("SceneData", m_sceneDataUBOs);
			// m_geometryPass->setInput("u_DiffuseIrradianceMap", m_specInfo.scene->m_sceneEnvironment.diffuseIrradianceMap);
			m_geometryPass->setInput("u_AOTexture", m_aoBlurredOutputImage);

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
		_renderDepthPrePass(p_cmd);
		_renderAOPass(p_cmd);
		// _renderLightCullingPass(p_cmd);
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

	auto SceneRenderer::getMSAAOutputGeometryPositionsImage() -> gpu::RawImageHandle &
	{
		return m_geometryPositionsAttachmentImage;
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

	auto SceneRenderer::getResolveOutputGeometryPositionsTexture() const -> const gpu::Texture2DHandle &
	{
		return m_geometryPositionsResolveAttachmentTexture;
	}

	auto SceneRenderer::getSSAONoiseTexture() const -> const gpu::Texture2DHandle &
	{
		return m_ssaoNoiseTexture;
	}

	auto SceneRenderer::getOutputAOTexture() const -> const gpu::Texture2DHandle &
	{
		return m_ssaoOutputTexture;
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

			m_geometryPositionsAttachmentImage->resize(p_width, p_height);
			m_geometryPositionsResolveAttachmentTexture->resize(p_width, p_height);

			m_ssaoOutputTexture->resize(p_width, p_height);
			m_aoBlurredOutputImage->resize(p_width, p_height);

			m_colourImage->resize(p_width, p_height);
			m_resolveColourTexture->resize(p_width, p_height);

			m_renderer2D->onResize(p_width, p_height);
		}
	}

	auto SceneRenderer::reloadEnvironmentMaps(const gpu::Texture3DHandle &p_skybox, const gpu::Texture3DHandle &p_diffuse_irradiance) -> void
	{
		m_skyboxPass->setInput("u_CubemapImage", p_skybox);
		m_geometryPass->setInput("u_DiffuseIrradianceMap", p_diffuse_irradiance);
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

		gpu::RenderingAttachmentInfo &geo_positions_attachment_info{rendering_info.colourAttachments.emplace_back()};
		geo_positions_attachment_info.clearValue   = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};
		geo_positions_attachment_info.image        = m_geometryPositionsAttachmentImage;
		geo_positions_attachment_info.loadOp       = vk::AttachmentLoadOp::eClear;
		geo_positions_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
		geo_positions_attachment_info.resolveImage = m_geometryPositionsResolveAttachmentTexture->getImage();
		geo_positions_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

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
		glm::vec2 noise_scale{
			static_cast<float32>(m_specInfo.viewportWidth) / static_cast<float32>(m_ssaoNoiseTexture->getSpecInfo().width),
			static_cast<float32>(m_specInfo.viewportHeight) / static_cast<float32>(m_ssaoNoiseTexture->getSpecInfo().height)
		};
		m_aoFrameDataMaterial->set(".u_NoiseScale", noise_scale);

		m_ssaoPass->setInput("u_PositionsTex", m_geometryPositionsResolveAttachmentTexture);
		m_ssaoPass->setInput("u_NormalsTex", m_geometryNormalsResolveAttachmentTexture);

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &out_ssao_attachment_info{rendering_info.colourAttachments.emplace_back()};
		out_ssao_attachment_info.clearValue = vk::ClearColorValue{1.0f, 1.0, 1.0f, 1.0f};
		out_ssao_attachment_info.image      = m_ssaoOutputTexture->getImage();

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_ssaoPass);
		m_renderCtx->renderFullscreenQuad(p_cmd, m_ssaoPipeline, m_aoFrameDataMaterial);
		m_renderCtx->endRendering(p_cmd, rendering_info);

		const uint32     work_groups_x{(m_specInfo.viewportWidth + 15u) / 16u};
		const uint32     work_groups_y{(m_specInfo.viewportHeight + 15u) / 16u};
		constexpr uint32 work_groups_z{1u};
		m_renderCtx->beginCompute(p_cmd, m_aoBlurPass);
		m_renderCtx->dispatchCompute(p_cmd, m_aoBlurPass, nullptr, work_groups_x, work_groups_y, work_groups_z);
	}

	auto SceneRenderer::_renderLightCullingPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		// m_renderCtx->beginCompute(p_cmd, m_lightCullingPass);
		// m_renderCtx->dispatchCompute(p_cmd, m_lightCullingPass, m_lightCullingMaterial, m_specInfo.viewportWidth, m_specInfo.viewportHeight, 1);
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

	SceneRenderer::SSAOKernel::SSAOKernel()
	{
		static std::uniform_real_distribution<float32> s_random_floats{0.0f, 1.0f};
		static std::random_device                      s_device{};
		static std::default_random_engine              s_generator{s_device()};

		for (uint32 i{0u}; i < c_SSAOSampleCount; ++i)
		{
			glm::vec3 sample{s_random_floats(s_generator) * 2.0f - 1.0f, s_random_floats(s_generator) * 2.0f - 1.0f, s_random_floats(s_generator)};
			sample = glm::normalize(sample);
			sample *= s_random_floats(s_generator);

			float32 scale{static_cast<float32>(i) / static_cast<float32>(c_SSAOSampleCount)};
			scale  = glm::mix(0.1f, 1.0f, scale * scale);
			sample *= scale;

			samples[i] = {sample, 0.0f};
		}
	}
}
