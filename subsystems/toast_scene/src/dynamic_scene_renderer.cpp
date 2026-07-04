#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"
#include "toast_scene/dynamic_scene_renderer.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

#include <random>

#include "toast_lib/os/terminal.hpp"
#include "toast_render/globals.hpp"
#include "toast_scene/entity.hpp"

#define TST_ENABLE_2D_SCENE_RENDERING 0
#define TST_ENABLE_3D_SCENE_RENDERING 1

namespace toaster
{
	DynamicSceneRenderer::DynamicSceneRenderer(Scene *p_scene, tsm::uint2 p_viewport_size) : m_scene(p_scene), m_renderCtx(p_scene->m_renderCtx),
																							 m_viewportSize(p_viewport_size)
	{
		m_cameraUBOs    = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(render::Globals::CameraUB));
		m_sceneDataUBOs = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(SceneDataUB));

		m_colourRenderTarget = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor);
		m_depthRenderTarget  = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eDepth);

		m_depthPreGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();
		m_depthPreGraphicsState->setShaders({m_renderCtx->getGlobals()->getShader("Depth_Pre_VS"), m_renderCtx->getGlobals()->getShader("Depth_Pre_PS")}).
				setAttachmentCount(0u).setCullMode(vk::CullModeFlagBits::eBack).setEnableDepthTest(true).setEnableDepthWrite(true).setEnableMultisample(false);

		m_skyboxPass = m_renderCtx->createUnique<render::SkyboxPass>(m_scene->m_sceneEnvironment.skyboxMap);

		m_geometryGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();
		m_geometryGraphicsState->setShaders({m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_VS"), m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_PS")}).
				setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eBack).setEnableDepthTest(true).setEnableDepthWrite(false).setEnableMultisample(false).
				setEnableDepthCompareOp(vk::CompareOp::eLessOrEqual);
	}

	DynamicSceneRenderer::~DynamicSceneRenderer()
	{
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
			auto       output_colour_image{m_colourRenderTarget->getImage()};
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
		// m_scene->m_lightEnvironment.pointLights.clear();
		// m_scene->m_lightEnvironment.directionalLights.clear();
		// #if TST_ENABLE_3D_SCENE_RENDERING
		// {
		// 	for (const auto group{m_scene->m_registry.group(entt::get<DirectionalLightComponent>)}; const auto entity: group)
		// 	{
		// 		auto directional_light{group.get<DirectionalLightComponent>(entity)};
		//
		// 		Entity e{entity, m_scene};
		// 		auto   tc{m_scene->getEntityWorldTransformComponent(e)};
		//
		// 		Dx::XMVECTOR orientation{Dx::XMLoadFloat4(&tc.orientation)};
		// 		Dx::XMVECTOR direction{Dx::XMVector3Rotate(Dx::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), orientation)};
		//
		// 		auto &dir_light{m_scene->m_lightEnvironment.directionalLights.emplace_back()};
		// 		Dx::XMStoreFloat3(&dir_light.direction, direction);
		// 		dir_light.radiance   = directional_light.radiance;
		// 		dir_light.multiplier = directional_light.multiplier;
		// 	}
		// }
		// {
		// 	for (const auto view{m_scene->m_registry.view<PointLightComponent>()}; const auto entity: view)
		// 	{
		// 		auto   point_light{view.get<PointLightComponent>(entity)};
		// 		Entity e{entity, m_scene};
		// 		auto   tc{m_scene->getEntityWorldTransformComponent(e)};
		//
		// 		auto &light{m_scene->m_lightEnvironment.pointLights.emplace_back()};
		// 		light.position   = tc.translation;
		// 		light.radiance   = point_light.radiance;
		// 		light.multiplier = point_light.multiplier;
		// 	}
		// }
		//
		{
			if (m_scene->m_reloadEnvironment)
			{
				reloadEnvironmentMaps(m_scene->m_sceneEnvironment.skyboxMapImage, m_scene->m_sceneEnvironment.diffuseIrradianceMapImage);
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
		// #endif
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
		render::Globals::CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, p_view_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, p_projection_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.inverseProjectionMatrix, Dx::XMMatrixInverse(nullptr, p_projection_matrix));
		camera_ub.inverseProjectionMatrix.m[1][1] *= -1.0f; // I have to do ts...
		m_cameraUBOs->setData(camera_ub);

		#if 0
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
		#endif
		SceneDataUB scene_data_ub{};
		Dx::XMStoreFloat4(&scene_data_ub.cameraPos, p_camera_position);
		m_sceneDataUBOs->setData(scene_data_ub);
	}

	auto DynamicSceneRenderer::end() -> void
	{
		auto cmd{m_renderCtx->getCurrentCommandBuffer()};

		_performMeshTransformPrePass();
		_renderDepthPrePass(cmd);
		_renderSkyboxPass(cmd);
		_renderGeometryPass(cmd);

		m_submeshDrawCommands.clear();
	}

	auto DynamicSceneRenderer::renderMesh(const render::DynamicMeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void
	{
		MeshDrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh = p_mesh;
		Dx::XMStoreFloat4x4(&draw_command.transform, p_transform);
	}

	auto DynamicSceneRenderer::onResize(tsm::uint2 p_size) -> void
	{
		TST_ASSERT_MSG(p_size.x != 0 && p_size.y != 0, "Cannot resize to 0");

		if (m_viewportSize != p_size)
		{
			m_viewportSize = p_size;
			m_colourRenderTarget->resize(m_viewportSize);
			m_depthRenderTarget->resize(m_viewportSize);
		}
	}

	auto DynamicSceneRenderer::reloadEnvironmentMaps(const render::ImageHandle &p_skybox, const render::ImageHandle &p_diffuse_irradiance) -> void
	{
		m_skyboxPass->setEnvironmentMap(p_skybox);
		m_diffuseIrradianceMap = p_diffuse_irradiance;
	}

	auto DynamicSceneRenderer::_performMeshTransformPrePass() -> void
	{
		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			Dx::XMMATRIX transform{Dx::XMLoadFloat4x4(&draw_cmd.transform)};

			for (const auto &submesh: draw_cmd.mesh->getMeshData().submeshes)
			{
				auto &submesh_draw_cmd{m_submeshDrawCommands.emplace_back()};
				Dx::XMStoreFloat4x4(&submesh_draw_cmd.transform, Dx::XMMatrixMultiply(Dx::XMLoadFloat4x4(&submesh.transform), transform));

				submesh_draw_cmd.mesh = draw_cmd.mesh;

				submesh_draw_cmd.vertexOffset  = static_cast<int32>(submesh.vertexOffset);
				submesh_draw_cmd.vertexCount   = submesh.vertexCount;
				submesh_draw_cmd.indexOffset   = submesh.indexOffset;
				submesh_draw_cmd.indexCount    = submesh.indexCount;
				submesh_draw_cmd.materialIndex = submesh.materialIndex;
			}
		}

		m_meshDrawCommands.clear();
	}

	auto DynamicSceneRenderer::_renderDepthPrePass(gpu::CommandBuffer *p_cmd) -> void
	{
		if (m_submeshDrawCommands.empty())
			return;

		render::RenderingInfo rendering_info{m_viewportSize};
		rendering_info.setDepthAttachment(*m_depthRenderTarget->getImage());

		m_depthPreGraphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		DepthPreConstants depth_pre_constants{};
		depth_pre_constants.cameraPtr = m_cameraUBOs->getDeviceAddress();

		for (const auto &draw_cmd: m_submeshDrawCommands)
		{
			depth_pre_constants.meshTransform = draw_cmd.transform;
			depth_pre_constants.vertexBuffer  = draw_cmd.mesh->getVertexBufferAddress();

			p_cmd->pushData(depth_pre_constants);
			p_cmd->bindIndexBuffer(draw_cmd.mesh->getIndexBuffer());
			p_cmd->drawIndexed(draw_cmd.indexCount, 1, draw_cmd.indexOffset, draw_cmd.vertexOffset, 0);
		}
		m_renderCtx->endRendering(rendering_info, p_cmd);
	}

	auto DynamicSceneRenderer::_renderSkyboxPass(gpu::CommandBuffer *p_cmd) -> void
	{
		render::RenderingInfo rendering_info{m_viewportSize};
		rendering_info.addColourAttachment(*m_colourRenderTarget->getImage());

		m_skyboxPass->onRender(*p_cmd, m_cameraUBOs->getDeviceAddress(), rendering_info);
	}

	auto DynamicSceneRenderer::_renderGeometryPass(gpu::CommandBuffer *p_cmd) -> void
	{
		if (m_submeshDrawCommands.empty())
			return;

		render::RenderingInfo rendering_info{m_viewportSize};
		rendering_info.addColourAttachment(*m_colourRenderTarget->getImage(), render::EAttachmentUsageOP::eLoadStore);
		rendering_info.setDepthAttachment(*m_depthRenderTarget->getImage(), render::EAttachmentUsageOP::eLoadDontCare);
		rendering_info.depthReadOnly = true;

		auto &vk_cmd{p_cmd->getVulkanCommandBuffer()};

		p_cmd->bindShaders({m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_VS").get(), m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_PS").get()});

		p_cmd->setPrimitiveTopology(gpu::EPrimitiveTopology::eTriangleList);
		p_cmd->setCullMode(gpu::ECullMode::eBack);
		p_cmd->setFrontFace(gpu::EFrontFace::eCCW);

		p_cmd->setPolygonMode(gpu::EPolygonMode::eFill);

		// set the rasterization state
		// vk_cmd.setDepthClampEnableEXT(false);
		// vk_cmd.setDepthBiasEnableEXT(false);
		// vk_cmd.setRasterizerDiscardEnableEXT(false);
		// vk_cmd.setLineWidth(1.0f);

		// vk_cmd.setColorBlendEnableEXT(0, {false});
		// vk_cmd.setColorBlendEquationEXT(0, vk::ColorBlendEquationEXT{});
		// vk_cmd.setColorWriteMaskEXT(0, {
		// 								{
		// 									vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
		// 									vk::ColorComponentFlagBits::eA
		// 								}
		// 							});

		p_cmd->setDepthTestEnable(true);
		p_cmd->setDepthWriteEnable(false);
		p_cmd->setDepthCompareOp(gpu::ECompareOp::eLessOrEqual);

		p_cmd->setStencilTestEnable(false);

		vk_cmd.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xFFFFFFFF);
		vk_cmd.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);

		m_renderCtx->beginRendering(rendering_info, p_cmd);

		MeshDrawConstants mesh_draw_constants{};

		mesh_draw_constants.cameraPtr    = m_cameraUBOs->getDeviceAddress();
		mesh_draw_constants.sceneDataPtr = m_sceneDataUBOs->getDeviceAddress();

		mesh_draw_constants.samplerIndex              = m_renderCtx->getSampler(render::ESamplerType::eIrradianceMap);
		mesh_draw_constants.diffuseIrradianceMapIndex = m_diffuseIrradianceMap->getAlignedShaderReadHeapID();

		for (const auto &draw_cmd: m_submeshDrawCommands)
		{
			mesh_draw_constants.meshTransform = draw_cmd.transform;

			mesh_draw_constants.vertexBuffer = draw_cmd.mesh->getVertexBufferAddress();
			mesh_draw_constants.material     = draw_cmd.mesh->getMaterial(draw_cmd.materialIndex)->getDeviceAddress();

			p_cmd->pushData(mesh_draw_constants);
			p_cmd->bindIndexBuffer(draw_cmd.mesh->getIndexBuffer());
			p_cmd->drawIndexed(draw_cmd.indexCount, 1, draw_cmd.indexOffset, draw_cmd.vertexOffset, 0);
		}

		m_renderCtx->endRendering(rendering_info, p_cmd);
	}
}
