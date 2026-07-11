#include "toast_scene/scene_renderer.hpp"

#include "toast_render/globals.hpp"
#include "toast_render/render_context.hpp"

#include <random>

#include "toast_lib/os/terminal.hpp"
#include "toast_scene/entity.hpp"

namespace toaster::scene
{
	SceneRenderer::SceneRenderer(Scene &p_scene, tsm::uint2 p_initial_viewport_size) : m_scene(&p_scene), m_renderCtx(p_scene.getRenderCtx()),
																					   m_viewportSize(p_initial_viewport_size)
	{
		m_cameraUBOs    = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(render::Globals::CameraUB));
		m_sceneDataUBOs = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(SceneDataUB));

		LOG_INFO("Slot: {} | Heap ID: {}", m_sceneDataUBOs->getHeapID(), m_sceneDataUBOs->getAlignedHeapID());

		m_pointLightSSBOs = m_renderCtx->createUnique<render::StorageBufferPFF>(sizeof(PointLightSSBO));

		m_colourRenderTarget = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eColor);
		m_depthRenderTarget  = m_renderCtx->createAttachmentImage(m_viewportSize, vk::ImageAspectFlagBits::eDepth);
	}

	SceneRenderer::~SceneRenderer()
	{
	}

	auto SceneRenderer::onRender() -> void
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

	auto SceneRenderer::onRender(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
	{
		_begin(p_camera_position, p_view_matrix, p_projection_matrix);
		for (const auto view{m_scene->getRegistry().view<DynamicMeshComponent>()}; const auto entity: view)
		{
			const auto &mesh_comp{view.get<DynamicMeshComponent>(entity)};

			auto mesh{mesh_comp.mesh};
			if (mesh)
			{
				Entity e{entity, const_cast<Scene *>(m_scene)};

				submitMesh(mesh, m_scene->getEntityWorldTransformMatrix(e));
			}
		}
		_end();
	}

	auto SceneRenderer::submitMesh(const render::DynamicMeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void
	{
		MeshDrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh = p_mesh;
		Dx::XMStoreFloat4x4(&draw_command.transform, p_transform);
	}

	auto SceneRenderer::onResize(tsm::uint2 p_size) -> void
	{
		TST_ASSERT_MSG(p_size.x != 0 && p_size.y != 0, "Cannot resize to 0");

		if (m_viewportSize != p_size)
		{
			m_viewportSize = p_size;
			m_colourRenderTarget->resize(m_viewportSize);
			m_depthRenderTarget->resize(m_viewportSize);
		}
	}

	auto SceneRenderer::_begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void
	{
		render::Globals::CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, p_view_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, p_projection_matrix);
		Dx::XMStoreFloat4x4(&camera_ub.inverseProjectionMatrix, Dx::XMMatrixInverse(nullptr, p_projection_matrix));
		m_cameraUBOs->copyData(camera_ub);

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
		m_sceneDataUBOs->copyData(scene_data_ub);
	}

	auto SceneRenderer::_end() -> void
	{
		auto &cmd{*m_renderCtx->getCurrentCommandBuffer()};

		_buildDrawCommands();
		_setRequiredRenderState(cmd);
		_renderDepthPrePass(cmd);
		_renderSkyboxPass(cmd);

		_renderGeometryPass(cmd);

		m_submeshDrawCommands.clear();
	}

	auto SceneRenderer::_buildDrawCommands() -> void
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

		std::ranges::sort(m_submeshDrawCommands, [](const SubmeshDrawCommand &lhs, const SubmeshDrawCommand &rhs) -> bool
		{
			return lhs.mesh->getIndexBufferAddress() < rhs.mesh->getIndexBufferAddress();
		});

		m_meshDrawCommands.clear();
	}

	auto SceneRenderer::_setRequiredRenderState(gpu::CommandBuffer &p_cmd) -> void
	{
		// As I am using BDA, there is no need to use any vertex input state. But I still have to set it
		p_cmd.getVulkanCommandBuffer().setVertexInputEXT({}, {});

		p_cmd.setPrimitiveTopology(gpu::EPrimitiveTopology::eTriangleList);
		p_cmd.setFrontFace(gpu::EFrontFace::eCCW);
		p_cmd.setPolygonMode(gpu::EPolygonMode::eFill);

		p_cmd.setAlphaToCoverageEnable(false);
		p_cmd.setPrimitiveRestartEnable(false);
		p_cmd.setStencilTestEnable(false);
		p_cmd.setLineWidth(1.0f);

		p_cmd.setDepthBiasEnable(false);
		p_cmd.setDepthClampEnable(false);
		p_cmd.setRasterizerDiscardEnable(false);

		p_cmd.setRasterizationSamples(gpu::ESampleCount::e1);
		p_cmd.setDepthCompareOp(gpu::ECompareOp::eLessOrEqual);
	}

	auto SceneRenderer::_renderDepthPrePass(gpu::CommandBuffer &p_cmd) -> void
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
			// depth_pre_constants.indexBuffer   = draw_cmd.mesh->getIndexBufferAddress();

			// depth_pre_constants.indexOffset = draw_cmd.indexOffset;

			p_cmd.bindIndexBuffer(draw_cmd.mesh->getIndexBuffer());

			p_cmd.pushData(depth_pre_constants);
			p_cmd.drawIndexed(draw_cmd.indexCount, 1, draw_cmd.indexOffset, draw_cmd.vertexOffset, 0);
		}
		m_renderCtx->endRendering(rendering_info, &p_cmd);
	}

	auto SceneRenderer::_renderSkyboxPass(gpu::CommandBuffer &p_cmd) -> void
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
		skybox_constants.environmentMapAddressOffset = m_scene->getSkyboxMap()->getAlignedShaderReadHeapID();
		p_cmd.pushData(skybox_constants);

		m_renderCtx->renderFullscreenQuad();
		m_renderCtx->endRendering(rendering_info, &p_cmd);
	}

	auto SceneRenderer::_renderGeometryPass(gpu::CommandBuffer &p_cmd) -> void
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

		mesh_draw_constants.cameraIndex    = m_cameraUBOs->getAlignedHeapID();
		mesh_draw_constants.sceneDataIndex = m_sceneDataUBOs->getAlignedHeapID();

		mesh_draw_constants.pointLightsPtr = m_pointLightSSBOs->getDeviceAddress();

		mesh_draw_constants.samplerIndex               = m_renderCtx->getSampler(render::ESamplerType::eIrradianceMap);
		mesh_draw_constants.diffuseIrradianceMapIndex  = m_scene->getDiffuseIrradianceMap()->getAlignedShaderReadHeapID();
		mesh_draw_constants.specularIrradianceMapIndex = m_scene->getSpecularIrradianceMap()->getAlignedShaderReadHeapID();

		mesh_draw_constants.BRDFLUTSamplerIndex = m_renderCtx->getSampler(render::ESamplerType::eBRDFLUT);
		mesh_draw_constants.BRDFLUT             = m_renderCtx->getGlobals()->BRDFLUT()->getAlignedShaderReadHeapID();

		render::DynamicMeshHandle last_mesh{nullptr};
		gpu::ShaderHandle         last_vs{nullptr};
		gpu::ShaderHandle         last_ps{nullptr};
		for (const auto &draw_cmd: m_submeshDrawCommands)
		{
			const auto &material{draw_cmd.mesh->getMaterial(draw_cmd.materialIndex)};

			if (last_vs.get() != material->getVertexShader().get() || last_ps.get() != material->getPixelShader().get())
			{
				last_vs = material->getVertexShader();
				last_ps = material->getPixelShader();
				p_cmd.bindShaders({last_vs, last_ps});
			}

			mesh_draw_constants.meshTransform = draw_cmd.transform;

			mesh_draw_constants.vertexBuffer = draw_cmd.mesh->getVertexBufferAddress();

			mesh_draw_constants.materialIndex = material->getHeapID();

			p_cmd.pushData(mesh_draw_constants);

			if (draw_cmd.mesh.get() != last_mesh)
			{
				last_mesh = draw_cmd.mesh;
				p_cmd.bindIndexBuffer(last_mesh->getIndexBuffer());
			}

			if (material->flags & render::EMaterialPropertyFlags::eTwoSided)
				p_cmd.setCullMode(gpu::ECullMode::eNone);

			if (material->flags & render::EMaterialPropertyFlags::eWireframe)
				p_cmd.setPolygonMode(gpu::EPolygonMode::eLine);

			p_cmd.drawIndexed(draw_cmd.indexCount, 1, draw_cmd.indexOffset, draw_cmd.vertexOffset, 0);

			if (material->flags & render::EMaterialPropertyFlags::eTwoSided)
				p_cmd.setCullMode(gpu::ECullMode::eBack);

			if (material->flags & render::EMaterialPropertyFlags::eWireframe)
				p_cmd.setPolygonMode(gpu::EPolygonMode::eFill);
		}

		m_renderCtx->endRendering(rendering_info, &p_cmd);
	}
}
