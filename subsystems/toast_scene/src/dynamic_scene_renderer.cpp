#include "toast_scene/dynamic_scene_renderer.hpp"
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

		m_pointLightSSBOs = m_renderCtx->createUnique<render::StorageBufferPFF>(sizeof(PointLightSSBO));

		m_colourRenderTarget = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor);
		m_depthRenderTarget  = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eDepth);
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
		m_scene->m_lightEnvironment.pointLights.clear();
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
		for (const auto view{m_scene->m_registry.view<PointLightComponent>()}; const auto entity: view)
		{
			auto   point_light{view.get<PointLightComponent>(entity)};
			Entity e{entity, m_scene};
			auto   tc{m_scene->getEntityWorldTransformComponent(e)};

			auto &light{m_scene->m_lightEnvironment.pointLights.emplace_back()};
			light.position          = {tc.translation.x, tc.translation.y, tc.translation.z, 1.0f};
			light.radianceIntensity = {point_light.radiance, point_light.multiplier};
		}
		// }
		//
		if (m_scene->m_reloadEnvironment)
		{
			reloadEnvironmentMaps(m_scene->m_sceneEnvironment.diffuseIrradianceMapImage, m_scene->m_sceneEnvironment.specularIrradianceMapImage);
			m_scene->m_reloadEnvironment = false;
		}

		begin(p_camera_position, p_view_matrix, p_projection_matrix);
		for (const auto view{m_scene->m_registry.view<DynamicMeshComponent>()}; const auto entity: view)
		{
			const auto &mesh_comp{view.get<DynamicMeshComponent>(entity)};

			auto mesh{mesh_comp.mesh};
			if (mesh)
			{
				Entity e{entity, m_scene};

				renderMesh(mesh, m_scene->getEntityWorldTransformMatrix(e));
			}
		}
		end();
	}

	auto DynamicSceneRenderer::begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
	{
		render::Globals::CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, p_view_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, p_projection_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.inverseProjectionMatrix, Dx::XMMatrixInverse(nullptr, p_projection_matrix));
		m_cameraUBOs->setData(camera_ub);

		const auto &   point_lights{m_scene->getLightEnvironment().pointLights};
		PointLightSSBO point_light_ssbo{};
		point_light_ssbo.count = std::min(static_cast<uint32>(point_lights.size()), PointLightSSBO::maxPointLights);
		for (uint32 i{0u}; i < PointLightSSBO::maxPointLights && i < point_lights.size(); ++i)
		{
			point_light_ssbo.pointLights[i].position          = point_lights[i].position;
			point_light_ssbo.pointLights[i].radianceIntensity = point_lights[i].radianceIntensity;
		}
		m_pointLightSSBOs->setData(point_light_ssbo);

		SceneDataUB scene_data_ub{};
		Dx::XMStoreFloat4(&scene_data_ub.cameraPos, p_camera_position);
		m_sceneDataUBOs->setData(scene_data_ub);
	}

	auto DynamicSceneRenderer::end() -> void
	{
		auto &cmd{*m_renderCtx->getCurrentCommandBuffer()};

		_buildDrawCommands();
		_setRequiredRenderState(cmd);
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

	auto DynamicSceneRenderer::reloadEnvironmentMaps(const render::ImageHandle &p_diffuse_irradiance, const render::ImageHandle &p_specular_irradiance) -> void
	{
		m_diffuseIrradianceMap  = p_diffuse_irradiance;
		m_specularIrradianceMap = p_specular_irradiance;
	}

	auto DynamicSceneRenderer::_buildDrawCommands() -> void
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
				submesh_draw_cmd.indexOffset   = submesh.indexOffset;
				submesh_draw_cmd.indexCount    = submesh.indexCount;
				submesh_draw_cmd.materialIndex = submesh.materialIndex;
			}
		}

		// std::ranges::sort(m_submeshDrawCommands, [](const SubmeshDrawCommand &lhs, const SubmeshDrawCommand &rhs) -> bool
		// {
		// return false;
		// });

		m_meshDrawCommands.clear();
	}

	auto DynamicSceneRenderer::_setRequiredRenderState(gpu::CommandBuffer &p_cmd) -> void
	{
		// As I am using BDA, there is no need to use any vertex input state. But I still have to set it
		p_cmd.getVulkanCommandBuffer().setVertexInputEXT({}, {});

		p_cmd.setPrimitiveTopology(gpu::EPrimitiveTopology::eTriangleList);
		p_cmd.setFrontFace(gpu::EFrontFace::eCCW);
		p_cmd.setPolygonMode(gpu::EPolygonMode::eFill);

		p_cmd.getVulkanCommandBuffer().setPrimitiveRestartEnableEXT(false);
		p_cmd.getVulkanCommandBuffer().setAlphaToCoverageEnableEXT(false);

		p_cmd.setStencilTestEnable(false);
		p_cmd.getVulkanCommandBuffer().setLineWidth(1.0f);

		p_cmd.getVulkanCommandBuffer().setDepthClampEnableEXT(false);
		p_cmd.getVulkanCommandBuffer().setDepthBiasEnableEXT(false);
		p_cmd.getVulkanCommandBuffer().setRasterizerDiscardEnableEXT(false);

		p_cmd.getVulkanCommandBuffer().setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xFFFFFFFF);
		p_cmd.getVulkanCommandBuffer().setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);

		p_cmd.setDepthCompareOp(gpu::ECompareOp::eLessOrEqual);
	}

	auto DynamicSceneRenderer::_renderDepthPrePass(gpu::CommandBuffer &p_cmd) -> void
	{
		if (m_submeshDrawCommands.empty())
			return;

		render::RenderingInfo rendering_info{m_viewportSize};
		rendering_info.setDepthAttachment(*m_depthRenderTarget->getImage());

		p_cmd.bindShaders({m_renderCtx->getGlobals()->getShader("Depth_Pre_VS"), m_renderCtx->getGlobals()->getShader("Depth_Pre_PS")});
		p_cmd.setCullMode(gpu::ECullMode::eBack);

		p_cmd.setDepthTestEnable(true);
		p_cmd.setDepthWriteEnable(true);

		m_renderCtx->beginRendering(rendering_info, &p_cmd);

		DepthPreConstants depth_pre_constants{};
		depth_pre_constants.cameraPtr = m_cameraUBOs->getDeviceAddress();

		for (const auto &draw_cmd: m_submeshDrawCommands)
		{
			depth_pre_constants.meshTransform = draw_cmd.transform;
			depth_pre_constants.vertexBuffer  = draw_cmd.mesh->getVertexBufferAddress();

			p_cmd.pushData(depth_pre_constants);
			p_cmd.bindIndexBuffer(draw_cmd.mesh->getIndexBuffer());
			p_cmd.drawIndexed(draw_cmd.indexCount, 1, draw_cmd.indexOffset, draw_cmd.vertexOffset, 0);
		}
		m_renderCtx->endRendering(rendering_info, &p_cmd);
	}

	auto DynamicSceneRenderer::_renderSkyboxPass(gpu::CommandBuffer &p_cmd) -> void
	{
		render::RenderingInfo rendering_info{m_viewportSize};
		rendering_info.addColourAttachment(*m_colourRenderTarget->getImage());

		p_cmd.bindShaders({m_renderCtx->getGlobals()->getShader("Skybox_VS"), m_renderCtx->getGlobals()->getShader("Skybox_PS")});
		p_cmd.setCullMode(gpu::ECullMode::eNone);
		p_cmd.setDepthTestEnable(false);

		p_cmd.setColourBlendEnable({false});
		p_cmd.setColourWriteMask({vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags});

		m_renderCtx->beginRendering(rendering_info, &p_cmd);

		SkyboxConstants skybox_constants{};
		skybox_constants.vertexBufferBDA             = m_renderCtx->getGlobals()->fullscreenQuadVertexBuffer().getDeviceAddress();
		skybox_constants.cameraBDA                   = m_cameraUBOs->getDeviceAddress();
		skybox_constants.samplerAddressOffset        = m_renderCtx->getSampler(render::ESamplerType::eIrradianceMap);
		skybox_constants.environmentMapAddressOffset = m_scene->m_sceneEnvironment.skyboxMapImage->getAlignedShaderReadHeapID();
		p_cmd.pushData(skybox_constants);

		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(rendering_info, &p_cmd);
	}

	auto DynamicSceneRenderer::_renderGeometryPass(gpu::CommandBuffer &p_cmd) -> void
	{
		if (m_submeshDrawCommands.empty())
			return;

		render::RenderingInfo rendering_info{m_viewportSize};
		rendering_info.addColourAttachment(*m_colourRenderTarget->getImage(), render::EAttachmentUsageOP::eLoadStore);
		rendering_info.setDepthAttachment(*m_depthRenderTarget->getImage(), render::EAttachmentUsageOP::eLoadDontCare);
		rendering_info.depthReadOnly = true;

		p_cmd.setCullMode(gpu::ECullMode::eBack);

		p_cmd.setDepthTestEnable(true);
		p_cmd.setDepthWriteEnable(false);

		m_renderCtx->beginRendering(rendering_info, &p_cmd);

		MeshDrawConstants mesh_draw_constants{};

		mesh_draw_constants.cameraPtr      = m_cameraUBOs->getDeviceAddress();
		mesh_draw_constants.sceneDataPtr   = m_sceneDataUBOs->getDeviceAddress();
		mesh_draw_constants.pointLightsPtr = m_pointLightSSBOs->getDeviceAddress();

		mesh_draw_constants.samplerIndex               = m_renderCtx->getSampler(render::ESamplerType::eIrradianceMap);
		mesh_draw_constants.diffuseIrradianceMapIndex  = m_diffuseIrradianceMap->getAlignedShaderReadHeapID();
		mesh_draw_constants.specularIrradianceMapIndex = m_specularIrradianceMap->getAlignedShaderReadHeapID();

		mesh_draw_constants.BRDFLUTSamplerIndex = m_renderCtx->getSampler(render::ESamplerType::eBRDFLUT);
		mesh_draw_constants.BRDFLUT             = m_renderCtx->getGlobals()->BRDFLUT()->getAlignedShaderReadHeapID();

		render::DynamicMeshHandle last_mesh{nullptr};
		gpu::DynamicShaderHandle  last_ps{nullptr};
		for (const auto &draw_cmd: m_submeshDrawCommands)
		{
			const auto &material{draw_cmd.mesh->getMaterial(draw_cmd.materialIndex)};

			const auto material_shader{draw_cmd.mesh->getMaterialShader(draw_cmd.mesh->getMaterialType(draw_cmd.materialIndex))};
			if (last_ps.get() != material_shader.get())
			{
				last_ps = material_shader;
				p_cmd.bindShaders({m_renderCtx->getGlobals()->getShader("Dynamic_Mesh_VS").get(), last_ps.get()});
			}

			mesh_draw_constants.meshTransform = draw_cmd.transform;

			mesh_draw_constants.vertexBuffer = draw_cmd.mesh->getVertexBufferAddress();
			mesh_draw_constants.material     = material->getDeviceAddress();

			p_cmd.pushData(mesh_draw_constants);
			if (draw_cmd.mesh.get() != last_mesh)
			{
				last_mesh = draw_cmd.mesh;
				p_cmd.bindIndexBuffer(last_mesh->getIndexBuffer());
			}

			if (material->flags & static_cast<uint64>(render::EMeshMaterialFlags::eTwoSided))
				p_cmd.setCullMode(gpu::ECullMode::eNone);

			p_cmd.drawIndexed(draw_cmd.indexCount, 1, draw_cmd.indexOffset, draw_cmd.vertexOffset, 0);

			if (material->flags & static_cast<uint64>(render::EMeshMaterialFlags::eTwoSided))
				p_cmd.setCullMode(gpu::ECullMode::eBack);
		}

		m_renderCtx->endRendering(rendering_info, &p_cmd);
	}
}
