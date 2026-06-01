#include "toast_scene/scene_renderer.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

#include <random>

#include "toast_scene/entity.hpp"

#define TST_ENABLE_2D_SCENE_RENDERING 1
#define TST_ENABLE_3D_SCENE_RENDERING 1

namespace toaster
{
	SceneRenderer::SceneRenderer(Scene *p_scene, const SceneRendererSpecInfo &p_spec_info) : m_scene(p_scene), m_renderCtx(p_scene->m_renderCtx), m_specInfo(p_spec_info)
	{
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
			depth_pre_pipeline_spec_info.cullMode          = m_specInfo.backfaceCulling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone;
			depth_pre_pipeline_spec_info.shader            = m_renderCtx->getGlobals()->shaderLibrary().get("Depth-Pre");
			m_depthPrePipeline                             = m_renderCtx->createGPURef<gpu::VKPipeline>(depth_pre_pipeline_spec_info, "Depth-Pre");

			m_depthPrePass = m_renderCtx->createGPURef<gpu::VKRenderPass>(m_depthPrePipeline);
			m_depthPrePass->setInput("Camera", m_cameraUBOs).bake();

			m_MSAADepthImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eDepth);
			m_depthTexture   = m_renderCtx->createAttachmentTexture(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eDepth);

			{
				m_MSAAGeometryNormalsImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor,
																						   vk::Format::eR16G16B16A16Sfloat);
				m_geometryNormalsTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor,
																				vk::Format::eR16G16B16A16Sfloat);
			}
			{
				m_MSAAGeometryPositionsImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor,
																							 vk::Format::eR16G16B16A16Sfloat);
				m_geometryPositionsTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor,
																				  vk::Format::eR16G16B16A16Sfloat);
			}
		}
		#pragma endregion

		#pragma region ambient occlusion
		{
			m_SSAOTexture = m_renderCtx->createAttachmentTexture(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);

			gpu::PipelineSpecInfo ssao_pipeline_spec_info{};
			ssao_pipeline_spec_info.vertexBufferLayout = {{gpu::EBufferDataType::eFloat3, "a_Position"}, {gpu::EBufferDataType::eFloat2, "a_TexCoord"}};
			ssao_pipeline_spec_info.colourAttachments  = {vk::Format::eR16G16B16A16Sfloat};
			ssao_pipeline_spec_info.shader             = m_renderCtx->getGlobals()->shaderLibrary().get("SSAO_Graphics");
			ssao_pipeline_spec_info.polygonMode        = vk::PolygonMode::eFill;
			ssao_pipeline_spec_info.multisample        = false;
			ssao_pipeline_spec_info.depthTest          = false;
			ssao_pipeline_spec_info.cullMode           = vk::CullModeFlagBits::eBack;
			m_SSAOPipeline                             = m_renderCtx->createGPURef<gpu::VKPipeline>(ssao_pipeline_spec_info, "SSAO");

			m_SSAOKernel = make_unique<SSAOKernel>(); // Generates the random rotation vectors
			auto ssao_kernel_ubo{m_renderCtx->createGPURef<gpu::VKUniformBuffer>(sizeof(SSAOKernel))};
			ssao_kernel_ubo->setData(m_SSAOKernel->samples, sizeof(SSAOKernel), 0);

			constexpr uint32     s_noise_texture_side_size{4u};
			gpu::TextureSpecInfo noise_texture_spec_info{};
			noise_texture_spec_info.size               = {s_noise_texture_side_size};
			noise_texture_spec_info.format             = vk::Format::eR32G32B32A32Sfloat;
			noise_texture_spec_info.generateMips       = false;
			noise_texture_spec_info.samplerFilter      = vk::Filter::eNearest;
			noise_texture_spec_info.samplerAddressMode = vk::SamplerAddressMode::eRepeat;

			std::vector<tsm::float4> ssao_noise{_generateSSAONoise(s_noise_texture_side_size)};
			m_SSAONoiseTexture = m_renderCtx->createGPURef<gpu::VKTexture2D>(noise_texture_spec_info, ssao_noise.data(), ssao_noise.size() * sizeof(tsm::float4));

			m_SSAOPass = m_renderCtx->createGPURef<gpu::VKRenderPass>(m_SSAOPipeline);
			m_SSAOPass->setInput("Camera", m_cameraUBOs).setInput("SSAOKernel", ssao_kernel_ubo).setInput("u_NoiseTex", m_SSAONoiseTexture).bake();

			m_SSAOFrameDataMaterial = m_renderCtx->createRef<render::Material>(m_renderCtx->getGlobals()->shaderLibrary().get("SSAO_Graphics"), "SSAO");
			m_SSAOFrameDataMaterial->set(".u_Radius", 0.5f).set(".u_Bias", 0.025f);

			{
				gpu::ImageSpecInfo blurred_ao_image_spec_info{};
				blurred_ao_image_spec_info.size   = m_specInfo.viewportSize;
				blurred_ao_image_spec_info.format = vk::Format::eR16G16B16A16Sfloat;
				blurred_ao_image_spec_info.usage  = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled;
				m_SSAOBlurredImage                = m_renderCtx->createGPURef<gpu::VKStorageImage>(blurred_ao_image_spec_info);

				m_SSAOBlurPipeline = m_renderCtx->createGPURef<gpu::VKComputePipeline>(m_renderCtx->getGlobals()->shaderLibrary().get("SSAO_Blur"));
				m_SSAOBlurPass     = m_renderCtx->createGPURef<gpu::VKComputePass>(m_SSAOBlurPipeline);

				m_SSAOBlurPass->setInput("u_Occlusion", m_SSAOTexture).setInput("o_BlurredOcclusion", m_SSAOBlurredImage).bake();
			}
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
			m_skyboxPipeline                             = m_renderCtx->createGPURef<gpu::VKPipeline>(skybox_pipeline_spec_info, "Skybox");

			m_skyboxPass = m_renderCtx->createGPURef<gpu::VKRenderPass>(m_skyboxPipeline);
			m_skyboxPass->setInput("Camera", m_cameraUBOs).bake();

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
			geometry_pipeline_spec_info.cullMode          = m_specInfo.backfaceCulling ? vk::CullModeFlagBits::eBack : vk::CullModeFlagBits::eNone;
			geometry_pipeline_spec_info.shader            = m_renderCtx->getGlobals()->shaderLibrary().get("Geometry");
			m_geometryPipeline                            = m_renderCtx->createGPURef<gpu::VKPipeline>(geometry_pipeline_spec_info, "Geometry");

			m_geometryPass = m_renderCtx->createGPURef<gpu::VKRenderPass>(m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_cameraUBOs).setInput("DirectionalLightData", m_directionalLightUBOs).setInput("PointLightData", m_pointLightUBOs).
					setInput("SceneData", m_sceneDataUBOs).setInput("u_AOTexture", m_SSAOBlurredImage).bake();
		}
		#pragma endregion

		m_MSAAcolourImage = m_renderCtx->createMultisampleAttachmentImage(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor);
		m_colourTexture   = m_renderCtx->createAttachmentTexture(m_specInfo.viewportSize, vk::ImageAspectFlagBits::eColor);

		render::Renderer2DSpecInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetSize    = m_specInfo.viewportSize;
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

	auto SceneRenderer::onRender() -> void
	{
		onRender(m_renderCtx->getCurrentSwapchainCommandBuffer());
	}

	auto SceneRenderer::onRender(Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
	{
		onRender(m_renderCtx->getCurrentSwapchainCommandBuffer(), p_view_matrix, p_projection_matrix);
	}

	auto SceneRenderer::onRender(gpu::VKCommandBuffer *p_cmd) -> void
	{
		const Camera *main_camera{nullptr};

		Dx::XMMATRIX camera_transform;
		if (Entity main_camera_entity{m_scene->getMainCameraEntity()})
		{
			main_camera      = &main_camera_entity.getComponent<CameraComponent>().camera;
			camera_transform = main_camera_entity.getComponent<TransformComponent>().getTransform();
		}

		if (!main_camera)
		{
			// Fixes vulkan validation error messages...
			auto       output_colour_image{m_colourTexture->getImage()};
			const auto current_image_layout{output_colour_image->getCurrentImageLayout()};
			if (current_image_layout != vk::ImageLayout::eShaderReadOnlyOptimal)
				gpu::util::transitionImageLayout(output_colour_image, output_colour_image->getCurrentImageLayout(), vk::ImageLayout::eShaderReadOnlyOptimal);
			TST_PERMA_ASSERT(output_colour_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal);
			return;
		}

		onRender(p_cmd, Dx::XMMatrixInverse(nullptr, camera_transform), main_camera->getProjectionMatrix());
	}

	auto SceneRenderer::onRender(gpu::VKCommandBuffer *p_cmd, Dx::FXMMATRIX p_view, Dx::CXMMATRIX p_projection) -> void
	{
		m_scene->m_lightEnvironment.pointLights.clear();
		m_scene->m_lightEnvironment.directionalLights.clear();
		#if TST_ENABLE_3D_SCENE_RENDERING
		{
			for (const auto group{m_scene->m_registry.group(entt::get<DirectionalLightComponent>)}; const auto entity: group)
			{
				auto directional_light{group.get<DirectionalLightComponent>(entity)};

				Entity e{entity, m_scene};
				auto   tc{m_scene->getEntityWorldTransformComponent(e)};

				Dx::XMVECTOR orientation{Dx::XMLoadFloat4(&tc.orientation)};
				Dx::XMVECTOR direction{Dx::XMVector3Rotate(Dx::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), orientation)};

				auto &dir_light{m_scene->m_lightEnvironment.directionalLights.emplace_back()};
				Dx::XMStoreFloat3(&dir_light.direction, direction);
				dir_light.radiance   = directional_light.radiance;
				dir_light.multiplier = directional_light.multiplier;
			}
		}
		{
			for (const auto view{m_scene->m_registry.view<PointLightComponent>()}; const auto entity: view)
			{
				auto   point_light{view.get<PointLightComponent>(entity)};
				Entity e{entity, m_scene};
				auto   tc{m_scene->getEntityWorldTransformComponent(e)};

				auto &light{m_scene->m_lightEnvironment.pointLights.emplace_back()};
				light.position   = tc.translation;
				light.radiance   = point_light.radiance;
				light.multiplier = point_light.multiplier;
			}
		}

		{
			if (m_scene->m_reloadEnvironment)
			{
				reloadEnvironmentMaps(m_scene->m_sceneEnvironment.skyboxMap, m_scene->m_sceneEnvironment.diffuseIrradianceMap);
				m_scene->m_reloadEnvironment = false;
			}

			begin(p_view, p_projection);
			for (const auto view{m_scene->m_registry.view<MeshComponent>()}; const auto entity: view)
			{
				const auto &mesh_comp{view.get<MeshComponent>(entity)};

				auto mesh{mesh_comp.mesh};
				if (mesh)
				{
					Entity       e{entity, m_scene};
					Dx::XMMATRIX transform{m_scene->getEntityWorldTransformMatrix(e)};

					renderMesh(mesh, transform);
				}
			}
			end(p_cmd);
		}
		#endif
		#if TST_ENABLE_2D_SCENE_RENDERING
		{
			m_renderer2D->begin(p_view, p_projection);

			for (const auto view{m_scene->m_registry.view<SpriteRendererComponent>()}; const auto entity: view)
			{
				const auto &src{view.get<SpriteRendererComponent>(entity)};

				Entity       e{entity, m_scene};
				Dx::XMMATRIX transform{m_scene->getEntityWorldTransformMatrix(e)};

				auto texture{src.texture};
				if (texture)
					m_renderer2D->submitQuad(transform, texture, src.colour);
				else
					m_renderer2D->submitQuad(transform, src.colour);
			}

			gpu::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue   = vk::ClearColorValue{1.0f, 0.0f, 0.0f, 1.0f};
			colour_attachment_info.image        = m_MSAAcolourImage;
			colour_attachment_info.loadOp       = vk::AttachmentLoadOp::eNone;
			colour_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
			colour_attachment_info.resolveImage = m_colourTexture->getImage();
			colour_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eAverage;

			gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue   = vk::ClearDepthStencilValue{1.0f, 0u};
			depth_attachment_info.image        = m_MSAADepthImage;
			depth_attachment_info.loadOp       = vk::AttachmentLoadOp::eLoad;
			depth_attachment_info.storeOp      = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.resolveImage = m_depthTexture->getImage();
			depth_attachment_info.resolveMode  = vk::ResolveModeFlagBits::eMin;

			m_renderer2D->end(p_cmd, &colour_attachment_info, &depth_attachment_info);
		}
		#endif
	}

	auto SceneRenderer::begin(Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
	{
		uint32 frame_index{m_renderCtx->getCurrentFrameIndex()};

		CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.view, p_view_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.proj, p_projection_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.invProj, Dx::XMMatrixInverse(nullptr, p_projection_matrix));

		std::memcpy(m_mappedCameraUBOs[frame_index], &camera_ub, sizeof(CameraUB));

		const auto &[directional_lights, point_lights]{m_scene->getLightEnvironment()};
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

		Dx::XMVECTOR R{p_view_matrix.r[0]};
		Dx::XMVECTOR U{p_view_matrix.r[1]};
		Dx::XMVECTOR F{p_view_matrix.r[2]};
		Dx::XMVECTOR T{p_view_matrix.r[3]};

		Dx::XMVECTOR pos_x{Dx::XMVector3Dot(T, R)};
		Dx::XMVECTOR pos_y{Dx::XMVector3Dot(T, U)};
		Dx::XMVECTOR pos_z{Dx::XMVector3Dot(T, F)};

		Dx::XMVECTOR camera_pos{Dx::XMVectorSet(Dx::XMVectorGetX(pos_x), Dx::XMVectorGetY(pos_y), Dx::XMVectorGetZ(pos_z), 1.0f)};
		camera_pos = Dx::XMVectorScale(camera_pos, -1.0f);

		Dx::XMStoreFloat3(&scene_data_ub.cameraPos, camera_pos);
		std::memcpy(m_mappedSceneDataUBOs[frame_index], &scene_data_ub, sizeof(SceneDataUB));
	}

	auto SceneRenderer::end(gpu::VKCommandBuffer *p_cmd) -> void
	{
		_renderDepthPrePass(p_cmd);
		_renderAOPass(p_cmd);
		_renderSkyboxPass(p_cmd);
		_renderGeometryPass(p_cmd);

		m_meshDrawCommands.clear();
	}

	auto SceneRenderer::renderMesh(const render::MeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void
	{
		DrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh = p_mesh;
		Dx::XMStoreFloat4x4(&draw_command.transform, p_transform);
	}

	auto SceneRenderer::onResize(tsm::uint2 p_size) -> void
	{
		TST_ASSERT_MSG(p_size.x != 0 && p_size.y != 0, "Cannot resize to 0");

		if (m_specInfo.viewportSize != p_size)
		{
			m_specInfo.viewportSize = p_size;

			m_MSAADepthImage->resize(p_size);
			m_depthTexture->resize(p_size);

			m_MSAAGeometryNormalsImage->resize(p_size);
			m_geometryNormalsTexture->resize(p_size);

			m_MSAAGeometryPositionsImage->resize(p_size);
			m_geometryPositionsTexture->resize(p_size);

			m_SSAOTexture->resize(p_size);
			m_SSAOBlurredImage->resize(p_size);

			m_MSAAcolourImage->resize(p_size);
			m_colourTexture->resize(p_size);

			m_renderer2D->onResize(p_size);
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
		rendering_info.renderArea = m_renderCtx->getRenderingArea(m_specInfo.viewportSize);

		auto depth_attachment_info{m_renderCtx->getRenderingAttachmentInfo(*m_MSAADepthImage, *m_depthTexture->getImage())};
		rendering_info.depthAttachment = depth_attachment_info;

		auto &geo_normals_attachment_info{rendering_info.colourAttachments.emplace_back()};
		geo_normals_attachment_info = m_renderCtx->getRenderingAttachmentInfo(*m_MSAAGeometryNormalsImage, *m_geometryNormalsTexture->getImage());

		auto &geo_positions_attachment_info{rendering_info.colourAttachments.emplace_back()};
		geo_positions_attachment_info = m_renderCtx->getRenderingAttachmentInfo(*m_MSAAGeometryPositionsImage, *m_geometryPositionsTexture->getImage());

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_depthPrePass);
		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				Dx::XMMATRIX draw_cmd_transform{Dx::XMLoadFloat4x4(&draw_cmd.transform)};
				Dx::XMMATRIX submesh_transform{Dx::XMLoadFloat4x4(&draw_cmd.mesh->getSubmeshes()[i].localTransform)};
				m_renderCtx->renderMesh(p_cmd, draw_cmd.mesh, i, m_depthPrePipeline, Dx::XMMatrixMultiply(submesh_transform, draw_cmd_transform), nullptr);
			}
		}
		m_renderCtx->endRendering(p_cmd, rendering_info);
	}

	auto SceneRenderer::_renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		tsm::float2 noise_scale{static_cast<tsm::float2>(m_specInfo.viewportSize) / static_cast<tsm::float2>(m_SSAONoiseTexture->getSpecInfo().size)};
		m_SSAOFrameDataMaterial->set(".u_NoiseScale", noise_scale);
		m_SSAOPass->setInput("u_PositionsTex", m_geometryPositionsTexture).setInput("u_NormalsTex", m_geometryNormalsTexture);

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = m_renderCtx->getRenderingArea(m_specInfo.viewportSize);

		gpu::RenderingAttachmentInfo &out_ssao_attachment_info{rendering_info.colourAttachments.emplace_back()};
		out_ssao_attachment_info.clearValue = vk::ClearColorValue{1.0f, 1.0, 1.0f, 1.0f};
		out_ssao_attachment_info.image      = m_SSAOTexture->getImage();

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_SSAOPass);
		m_renderCtx->renderFullscreenQuad(p_cmd, m_SSAOPass, m_SSAOFrameDataMaterial);
		m_renderCtx->endRendering(p_cmd, rendering_info);

		m_renderCtx->beginCompute(p_cmd, m_SSAOBlurPass);
		m_renderCtx->dispatchCompute(p_cmd, m_SSAOBlurPass, nullptr, {(m_specInfo.viewportSize + 15u) / 16u, 1u});
	}

	auto SceneRenderer::_renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = m_renderCtx->getRenderingArea(m_specInfo.viewportSize);

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info = m_renderCtx->getRenderingAttachmentInfo(*m_MSAAcolourImage, *m_colourTexture->getImage());

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_skyboxPass);
		m_renderCtx->renderFullscreenQuad(p_cmd, m_skyboxPass, m_skyboxMaterial);
		m_renderCtx->endRendering(p_cmd, rendering_info);
	}

	auto SceneRenderer::_renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		if (m_meshDrawCommands.empty())
			return;

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea    = m_renderCtx->getRenderingArea(m_specInfo.viewportSize);
		rendering_info.depthReadOnly = true;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info = m_renderCtx->getRenderingAttachmentInfo(*m_MSAAcolourImage, *m_colourTexture->getImage(), gpu::EAttachmentUsageOP::eLoadStore);

		auto depth_attachment_info{m_renderCtx->getRenderingAttachmentInfo(*m_MSAADepthImage, *m_depthTexture->getImage(), gpu::EAttachmentUsageOP::eLoadDontCare)};
		rendering_info.depthAttachment = depth_attachment_info;

		m_renderCtx->beginRendering(p_cmd, rendering_info, m_geometryPass);
		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				Dx::XMMATRIX draw_cmd_transform{Dx::XMLoadFloat4x4(&draw_cmd.transform)};
				Dx::XMMATRIX submesh_transform{Dx::XMLoadFloat4x4(&draw_cmd.mesh->getSubmeshes()[i].localTransform)};
				m_renderCtx->renderMesh(p_cmd, draw_cmd.mesh, i, m_geometryPipeline, Dx::XMMatrixMultiply(submesh_transform, draw_cmd_transform));
			}
		}
		m_renderCtx->endRendering(p_cmd, rendering_info);
	}

	static std::uniform_real_distribution<float32> s_random_floats{0.0f, 1.0f};
	static std::random_device                      s_device{};
	static std::default_random_engine              s_generator{s_device()};

	auto SceneRenderer::_generateSSAONoise(uint32 p_texture_size) -> std::vector<tsm::float4>
	{
		std::vector<tsm::float4> ssaoNoise;

		for (uint32_t i = 0; i < p_texture_size * p_texture_size; i++)
		{
			float32 x{s_random_floats(s_generator) * 2.0f - 1.0f};
			float32 y{s_random_floats(s_generator) * 2.0f - 1.0f};
			float32 z{0.0f};

			auto &noise{ssaoNoise.emplace_back()};
			noise = {x, y, z, 1.0f};
		}
		return ssaoNoise;
	}

	SceneRenderer::SSAOKernel::SSAOKernel()
	{
		for (uint32 i{0u}; i < c_SSAOSampleCount; ++i)
		{
			Dx::XMVECTOR sample{
				Dx::XMVectorSet(s_random_floats(s_generator) * 2.0f - 1.0f, s_random_floats(s_generator) * 2.0f - 1.0f, s_random_floats(s_generator), 0.0f)
			};

			sample = Dx::XMVector2Normalize(sample);
			sample = Dx::XMVectorScale(sample, s_random_floats(s_generator));

			float32 scale{static_cast<float32>(i) / static_cast<float32>(c_SSAOSampleCount)};
			scale  = tsm::mix(0.1f, 1.0f, scale * scale);
			sample = Dx::XMVectorScale(sample, scale);

			Dx::XMStoreFloat4(&samples[i], sample);
		}
	}
}
