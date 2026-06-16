#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

#include <random>

#include "toast_lib/os/terminal.hpp"
#include "toast_scene/entity.hpp"

#define TST_ENABLE_2D_SCENE_RENDERING 0
#define TST_ENABLE_3D_SCENE_RENDERING 1

namespace toaster
{
	DynamicSceneRenderer::DynamicSceneRenderer(Scene *p_scene, tsm::uint2 p_viewport_size) : m_scene(p_scene), m_renderCtx(p_scene->m_renderCtx),
																							 m_viewportSize(p_viewport_size)
	{
		m_cameraUBOs = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(CameraUB));

		m_directionalLightUBOs = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(DirectionalLightUB));
		m_pointLightUBOs       = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(PointLightUB));
		m_sceneDataUBOs        = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(SceneDataUB));

		#pragma region depth-pre
		{
			m_depthPreGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();
			m_depthPreGraphicsState->setShaders({
													m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Depth_Pre_VS"),
													m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Depth_Pre_PS")
												}).setVertexBufferLayout(render::RenderContext::meshVbl).setAttachmentCount(3u).setCullMode(vk::CullModeFlagBits::eNone).
					setEnableDepthTest(true).setEnableDepthWrite(true).setEnableMultisample(true);

			m_MSAADepthImage = m_renderCtx->createMultisampleAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eDepth);
			m_depthImage     = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eDepth);

			m_MSAAGeometryNormalsImage = m_renderCtx->createMultisampleAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
			m_geometryNormalsImage     = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);

			m_MSAAGeometryPositionsImage = m_renderCtx->createMultisampleAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor,
																						 vk::Format::eR16G16B16A16Sfloat);
			m_geometryPositionsImage = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor, vk::Format::eR16G16B16A16Sfloat);
		}
		#pragma endregion

		#if 0
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

			m_SSAOPass = m_renderCtx->createRef<render::RenderPass>(m_SSAOPipeline);
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
				m_SSAOBlurPass     = m_renderCtx->createRef<render::ComputePass>(m_SSAOBlurPipeline);

				m_SSAOBlurPass->setInput("u_Occlusion", m_SSAOTexture).setInput("o_BlurredOcclusion", m_SSAOBlurredImage).bake();
			}
		}
		#pragma endregion
		#endif

		#pragma region skybox
		{
			m_skyboxGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();
			m_skyboxGraphicsState->setShaders({
												  m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Skybox_VS"),
												  m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Skybox_PS")
											  }).setVertexBufferLayout(render::RenderContext::fullscreenQuadVbl).setAttachmentCount(1u).
					setCullMode(vk::CullModeFlagBits::eNone).setEnableDepthTest(false).setEnableDepthWrite(false).setEnableMultisample(true);

			m_MSAAColourImage = m_renderCtx->createMultisampleAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor);
			m_colourImage     = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor);

			m_skyboxImage = m_renderCtx->getGlobals()->whiteImage();
		}
		#pragma endregion

		#pragma region geometry
		{
			m_geometryGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();
			m_geometryGraphicsState->setShaders({
													m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Mesh_Geo_VS"),
													m_renderCtx->getGlobals()->dynamicShaderLibrary().get("Mesh_Geo_PS")
												}).setVertexBufferLayout(render::RenderContext::meshVbl).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eNone).
					setEnableDepthTest(true).setEnableDepthWrite(false).setEnableMultisample(true).setEnableDepthCompareOp(vk::CompareOp::eLessOrEqual);
		}
		#pragma endregion
	}

	DynamicSceneRenderer::~DynamicSceneRenderer()
	{
		// m_sceneDataUBOs->unmapAllMemory();
		// m_directionalLightUBOs->unmapAllMemory();
		// m_cameraUBOs->unmapAllMemory();
	}

	auto DynamicSceneRenderer::onRender() -> void
	{
		const Camera *main_camera{nullptr};

		Dx::XMMATRIX camera_transform;
		Dx::XMVECTOR camera_positon;
		if (Entity main_camera_entity{m_scene->getMainCameraEntity()})
		{
			main_camera      = &main_camera_entity.getComponent<CameraComponent>().camera;
			camera_transform = main_camera_entity.getComponent<TransformComponent>().getTransform();
			camera_positon   = Dx::XMLoadFloat3(&main_camera_entity.getComponent<TransformComponent>().translation);
		}

		if (!main_camera)
		{
			LOG_WARN("Scene has no main camera!!!");

			// Fixes vulkan validation error messages...
			auto       output_colour_image{m_colourImage->getImage()};
			const auto current_image_layout{output_colour_image->getCurrentImageLayout()};
			if (current_image_layout != vk::ImageLayout::eShaderReadOnlyOptimal)
				gpu::util::transitionImageLayout(output_colour_image, output_colour_image->getCurrentImageLayout(), vk::ImageLayout::eShaderReadOnlyOptimal);
			TST_PERMA_ASSERT(output_colour_image->getCurrentImageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal);
			return;
		}
		Dx::XMMATRIX view_matrix{Dx::XMMatrixInverse(nullptr, camera_transform)};
		Dx::XMMATRIX correction{Dx::XMMatrixSet(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f)};

		view_matrix = Dx::XMMatrixMultiply(view_matrix, correction);
		onRender(camera_positon, view_matrix, main_camera->getProjectionMatrix());
	}

	auto DynamicSceneRenderer::onRender(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
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
				reloadEnvironmentMaps(m_scene->m_sceneEnvironment.skyboxMapImage, m_scene->m_sceneEnvironment.diffuseIrradianceMap);
				m_scene->m_reloadEnvironment = false;
			}

			begin(p_camera_position, p_view_matrix, p_projection_matrix);
			for (const auto view{m_scene->m_registry.view<DynamicMeshComponent>()}; const auto entity: view)
			{
				const auto &mesh_comp{view.get<DynamicMeshComponent>(entity)};

				auto mesh{mesh_comp.mesh};
				if (mesh)
				{
					Entity       e{entity, m_scene};
					Dx::XMMATRIX transform{m_scene->getEntityWorldTransformMatrix(e)};

					renderMesh(mesh, transform);
				}
			}
			end();
		}
		#endif
		#if TST_ENABLE_2D_SCENE_RENDERING
		{
			auto colour_attachment_info{render::getRenderingAttachmentInfo(*m_MSAAcolourImage, *m_colourTexture->getImage(), render::EAttachmentUsageOP::eNoneStore)};
			auto depth_attachment_info{render::getRenderingAttachmentInfo(*m_MSAADepthImage, *m_depthTexture->getImage(), render::EAttachmentUsageOP::eLoadStore)};

			m_renderer2D->begin(p_view, p_projection, &colour_attachment_info, &depth_attachment_info);
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
			m_renderer2D->end();
		}
		#endif
	}

	auto DynamicSceneRenderer::begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
	{
		uint32 frame_index{m_renderCtx->getCurrentFrameIndex()};

		CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.view, p_view_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.proj, p_projection_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.invProj, Dx::XMMatrixInverse(nullptr, p_projection_matrix));

		m_cameraUBOs->setData(camera_ub);

		const auto &[directional_lights, point_lights]{m_scene->getLightEnvironment()};
		{
			DirectionalLightUB directional_light_ub{};
			directional_light_ub.count = directional_lights.size();
			for (uint32 i{0u}; i < DirectionalLightUB::c_maxDirectionalLights && i < directional_lights.size(); ++i)
			{
				directional_light_ub.directionalLights[i].direction = directional_lights[i].direction;
				directional_light_ub.directionalLights[i].radiance  = directional_lights[i].radiance;
			}
			m_directionalLightUBOs->setData(directional_light_ub);
		}
		{
			PointLightUB point_light_ub{};
			point_light_ub.count = std::min(static_cast<uint32>(point_lights.size()), PointLightUB::c_maxPointLights);
			for (uint32 i{0u}; i < PointLightUB::c_maxPointLights && i < point_lights.size(); ++i)
			{
				point_light_ub.pointLights[i].position = point_lights[i].position;
				point_light_ub.pointLights[i].radiance = point_lights[i].radiance;
			}
			m_pointLightUBOs->setData(point_light_ub);
		}
		SceneDataUB scene_data_ub{};
		Dx::XMStoreFloat3(&scene_data_ub.cameraPos, p_camera_position);
		m_sceneDataUBOs->setData(scene_data_ub);
	}

	auto DynamicSceneRenderer::end() -> void
	{
		auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		_renderDepthPrePass(cmd);
		// _renderAOPass(cmd);
		_renderSkyboxPass(cmd);
		_renderGeometryPass(cmd);

		m_meshDrawCommands.clear();
	}

	auto DynamicSceneRenderer::renderMesh(const render::DynamicMeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void
	{
		DrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh = p_mesh;
		Dx::XMStoreFloat4x4(&draw_command.transform, p_transform);
	}

	auto DynamicSceneRenderer::onResize(tsm::uint2 p_size) -> void
	{
		TST_ASSERT_MSG(p_size.x != 0 && p_size.y != 0, "Cannot resize to 0");

		if (m_viewportSize != p_size)
		{
			m_viewportSize = p_size;

			m_MSAADepthImage->resize(p_size);
			m_depthImage->resize(p_size);

			m_MSAAGeometryNormalsImage->resize(p_size);
			m_geometryNormalsImage->resize(p_size);

			m_MSAAGeometryPositionsImage->resize(p_size);
			m_geometryPositionsImage->resize(p_size);

			m_MSAAColourImage->resize(p_size);
			m_colourImage->resize(p_size);
			// m_SSAOTexture->resize(p_size);
			// m_SSAOBlurredImage->resize(p_size);

			// m_MSAAcolourImage->resize(p_size);
			// m_colourTexture->resize(p_size);

			// m_renderer2D->onResize(p_size);
		}
	}

	auto DynamicSceneRenderer::reloadEnvironmentMaps(const render::ImageHandle &p_skybox, const render::ImageHandle &p_diffuse_irradiance) -> void
	{
		m_skyboxImage = p_skybox;

		// m_skyboxPass->setInput("u_CubeMapImage", p_skybox);
		// m_geometryPass->setInput("u_DiffuseIrradianceMap", p_diffuse_irradiance);
	}

	auto DynamicSceneRenderer::_renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		render::RenderingInfo rendering_info{};
		rendering_info.renderArea = render::getRenderingArea(m_viewportSize);

		auto depth_attachment_info{render::getRenderingAttachmentInfo(*m_MSAADepthImage, *m_depthImage->getImage())};
		rendering_info.depthAttachment = depth_attachment_info;

		auto &geo_normals_attachment_info{rendering_info.colourAttachments.emplace_back()};
		geo_normals_attachment_info = render::getRenderingAttachmentInfo(*m_MSAAGeometryNormalsImage, *m_geometryNormalsImage->getImage());

		auto &geo_positions_attachment_info{rendering_info.colourAttachments.emplace_back()};
		geo_positions_attachment_info = render::getRenderingAttachmentInfo(*m_MSAAGeometryPositionsImage, *m_geometryPositionsImage->getImage());

		m_depthPreGraphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		// p_cmd->setRenderArea(rendering_info.renderArea);

		DepthPreConstants depth_pre_constants{};
		depth_pre_constants.cameraPtr = m_cameraUBOs->getDeviceAddress();

		p_cmd->pushData(depth_pre_constants);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				auto &mesh{draw_cmd.mesh};

				// Dx::XMMATRIX draw_cmd_transform{Dx::XMLoadFloat4x4(&draw_cmd.transform)};
				// Dx::XMMATRIX submesh_transform{Dx::XMLoadFloat4x4(&mesh->getSubmeshes()[i].transform)};

				auto &submesh{mesh->getSubmeshes()[i]};
				p_cmd->pushData<Dx::XMFLOAT4X4>(submesh.transform, sizeof(DepthPreConstants));

				mesh->getVertexBuffer()->bind();
				mesh->getIndexBuffer()->bind();

				p_cmd->drawIndexed(submesh.indexCount, 1, submesh.baseIndex, static_cast<int32>(submesh.baseVertex), 0);
			}
		}

		// p_cmd->getVulkanCommandBuffer().endRendering();

		m_renderCtx->endRendering(rendering_info, p_cmd);
	}

	auto DynamicSceneRenderer::_renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		render::RenderingInfo rendering_info{};
		rendering_info.renderArea = render::getRenderingArea(m_viewportSize);

		render::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info = render::getRenderingAttachmentInfo(*m_MSAAColourImage, *m_colourImage->getImage());

		m_skyboxGraphicsState->bind(p_cmd);
		m_renderCtx->beginRendering(rendering_info, p_cmd);

		SkyboxConstants skybox_constants{};
		skybox_constants.cameraPtr = m_cameraUBOs->getDeviceAddress();
		skybox_constants.samplerId = m_renderCtx->getSampler(render::ESamplerType::eNearest);
		// skybox_constants.skyboxMapId = m_renderCtx->getGlobals()->whiteImage()->getAlignedHeapID();
		skybox_constants.skyboxMapId = m_skyboxImage->getAlignedShaderReadHeapID();
		p_cmd->pushData(skybox_constants);

		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(rendering_info, p_cmd);
	}

	auto DynamicSceneRenderer::_renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		if (m_meshDrawCommands.empty())
			return;

		render::RenderingInfo rendering_info{};
		rendering_info.renderArea    = render::getRenderingArea(m_viewportSize);
		rendering_info.depthReadOnly = true;

		render::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info = render::getRenderingAttachmentInfo(*m_MSAAColourImage, *m_colourImage->getImage(), render::EAttachmentUsageOP::eLoadStore);

		auto depth_attachment_info{render::getRenderingAttachmentInfo(*m_MSAADepthImage, *m_depthImage->getImage(), render::EAttachmentUsageOP::eLoadDontCare)};
		rendering_info.depthAttachment = depth_attachment_info;

		m_geometryGraphicsState->bind(p_cmd);
		m_renderCtx->beginRendering(rendering_info, p_cmd);

		GeometryConstants geometry_constants{};
		geometry_constants.cameraPtr            = m_cameraUBOs->getDeviceAddress();
		geometry_constants.directionalLightsPtr = m_directionalLightUBOs->getDeviceAddress();
		geometry_constants.pointLightsPtr       = m_pointLightUBOs->getDeviceAddress();
		geometry_constants.sceneDataPtr         = m_sceneDataUBOs->getDeviceAddress();
		p_cmd->pushData(geometry_constants);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				// Dx::XMMATRIX draw_cmd_transform{Dx::XMLoadFloat4x4(&draw_cmd.transform)};
				// Dx::XMMATRIX submesh_transform{Dx::XMLoadFloat4x4(&draw_cmd.mesh->getSubmeshes()[i].transform)};
				m_renderCtx->renderSubmesh(draw_cmd.mesh, i, sizeof(GeometryConstants), p_cmd);
			}
		}
		m_renderCtx->endRendering(rendering_info, p_cmd);
	}

	#if 0

	auto DynamicSceneRenderer::_renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void
	{
		tsm::float2 noise_scale{static_cast<tsm::float2>(m_specInfo.viewportSize) / static_cast<tsm::float2>(m_SSAONoiseTexture->getSpecInfo().size)};
		m_SSAOFrameDataMaterial->set(".u_NoiseScale", noise_scale);
		m_SSAOPass->setInput("u_PositionsTex", m_geometryPositionsTexture).setInput("u_NormalsTex", m_geometryNormalsTexture);

		render::RenderingInfo rendering_info{};
		rendering_info.renderArea = render::getRenderingArea(m_specInfo.viewportSize);

		render::RenderingAttachmentInfo &out_ssao_attachment_info{rendering_info.colourAttachments.emplace_back()};
		out_ssao_attachment_info.clearValue = vk::ClearColorValue{1.0f, 1.0, 1.0f, 1.0f};
		out_ssao_attachment_info.image      = m_SSAOTexture->getImage();

		m_renderCtx->beginRendering(rendering_info, m_SSAOPass, p_cmd);
		m_renderCtx->renderFullscreenQuad(m_SSAOPass, m_SSAOFrameDataMaterial, p_cmd);
		m_renderCtx->endRendering(rendering_info, p_cmd);

		m_renderCtx->beginCompute(m_SSAOBlurPass, p_cmd);
		m_renderCtx->dispatchCompute(m_SSAOBlurPass, nullptr, {(m_specInfo.viewportSize + 15u) / 16u, 1u}, p_cmd);
	} static std::uniform_real_distribution<float32> s_random_floats{0.0f, 1.0f}; static std::random_device s_device{}; static std::default_random_engine s_generator
			{s_device()}; auto                       DynamicSceneRenderer::_generateSSAONoise(uint32 p_texture_size) -> std::vector<tsm::float4>
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
	} DynamicSceneRenderer::SSAOKernel::SSAOKernel()
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
	#endif
}
