#include <toast_kernel/application.hpp>
#include <toast_kernel/fp_camera.hpp>
#include <toast_kernel/layer.hpp>

#include <toast_render/globals.hpp>
#include <toast_render/render_context.hpp>

#include <toast_lib/os/terminal.hpp>

#include "toast_kernel/input.hpp"
#include "toast_render/dynamic_mesh.hpp"
#include "toast_render/skybox_pass.hpp"

#include "toast_render/dynamic_material.hpp"

using namespace toaster;

struct NewMeshComponent
{
	NewMeshComponent()  = default;
	~NewMeshComponent() = default;

	RefPtr<render::DynamicMesh> mesh{nullptr};
};

class ClientLayer : public IAppLayer
{
public:
	struct SceneDataUB
	{
		Dx::XMFLOAT3 cameraPos;
		float32      _padd[1];
	};

	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		auto binary_dir{os::getBinaryDirectory()};
		auto resources_dir{binary_dir / "../resources"};

		m_MSAAColour = m_renderCtx->createMultisampleAttachmentImageUnique(m_viewportSize, vk::ImageAspectFlagBits::eColor, vk::Format::eR8G8B8A8Srgb);
		m_MSAADepth  = m_renderCtx->createMultisampleAttachmentImageUnique(m_viewportSize, vk::ImageAspectFlagBits::eDepth);

		m_scene = toaster::make_unique<Scene>(m_renderCtx);

		m_geometryGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();

		auto vertex_shader{m_globals->getShader("Dynamic_Mesh_VS")};
		auto pixel_shader{m_globals->getShader("Dynamic_Mesh_PS")};

		m_image = m_renderCtx->createImageRef(resources_dir / "textures/Peeber.png");

		auto reflection_data{render::reflection::reflectShader(*pixel_shader)};
		auto material_struct{render::findMaterialDeclaration(reflection_data)};
		TST_PERMA_ASSERT(material_struct);
		m_material = m_renderCtx->createUnique<render::DynamicMaterial>("New_Material", *material_struct);

		m_material->set("albedoMapIndex", m_image);
		m_material->set("samplerIndex", m_renderCtx->getSampler(render::ESamplerType::eDefault));

		m_geometryGraphicsState->setShaders({vertex_shader, pixel_shader}).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eBack).setEnableDepthTest(true).
				setEnableDepthWrite(true).setEnableMultisample(true);

		m_cameraUBOs    = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(render::Globals::CameraUB));
		m_sceneDataUBOs = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(SceneDataUB));

		m_environmentMap       = m_renderCtx->createEnvironmentMapImage(resources_dir / "environments/overcast_soil_puresky_2k.hdr");
		m_diffuseIrradianceMap = m_renderCtx->createDiffuseIrradianceMapImage(m_environmentMap);

		{
			Entity orbo_entity{m_scene->createEntity("Orbo")};
			auto & mmc{orbo_entity.addComponent<NewMeshComponent>()};
			mmc.mesh = m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/Orbo_Geo.fbx");
		}

		{
			Entity teapot_entity{m_scene->createEntity("Teapot")};
			auto & mmc{teapot_entity.addComponent<NewMeshComponent>()};
			mmc.mesh = m_renderCtx->createRef<render::DynamicMesh>(resources_dir / "meshes/utah_teapot.obj");

			auto mat{mmc.mesh->getMaterialData(0)};
			mat->roughness = 0.0f;

			teapot_entity.getTranslation() = {10.0f, 0.0f, 0.0f};
		}

		m_skyboxPass = m_renderCtx->createUnique<render::SkyboxPass>(m_viewportSize, m_environmentMap, true);
	}

	auto onDestroy() -> void override
	{
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		static float32 time{0.0f};
		time += p_dt;

		m_scene->onUpdate(p_dt);
		m_camera.onUpdate(p_dt);

		render::Globals::CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, m_camera.getViewMatrix());
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, m_camera.getProjectionMatrix());
		Dx::XMStoreFloat4x4(&camera_ub.inverseProjectionMatrix, Dx::XMMatrixInverse(nullptr, m_camera.getProjectionMatrix()));
		camera_ub.inverseProjectionMatrix.m[1][1] *= -1.0f; // I have to do ts...
		m_cameraUBOs->setData(camera_ub);

		auto       rendering_info{m_app->getWindow().getSwapchainRenderingInfo(true, {0.025f, 0.025f, 0.025f, 1.0f}, true)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &     vk_cmd{cmd->getVulkanCommandBuffer()};

		rendering_info.colourAttachments[0].image = m_MSAAColour.get();
		rendering_info.depthAttachment->image     = m_MSAADepth.get();

		m_skyboxPass->onRender(*cmd, m_cameraUBOs->getDeviceAddress(), rendering_info);

		rendering_info.colourAttachments[0].attachmentOp = render::EAttachmentUsageOP::eLoadStore;
		m_geometryGraphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);
		SceneDataUB scene_data_ub{};
		Dx::XMStoreFloat3(&scene_data_ub.cameraPos, m_camera.getPosition());
		m_sceneDataUBOs->setData(scene_data_ub);

		for (const auto view{m_scene->getRegistry().view<NewMeshComponent>()}; const auto entity: view)
		{
			const auto &mesh_comp{view.get<NewMeshComponent>(entity)};

			auto mesh{mesh_comp.mesh};
			if (mesh)
			{
				Entity       e{entity, m_scene.get()};
				Dx::XMMATRIX transform{m_scene->getEntityWorldTransformMatrix(e)};

				MeshletMeshConstants meshlet_mesh_constants{};
				meshlet_mesh_constants.vertexBuffer = mesh->getVertexBufferAddress();

				meshlet_mesh_constants.materialTest = m_material->getDeviceAddress();

				meshlet_mesh_constants.materialBuffer = mesh->getMaterialBufferAddress();

				meshlet_mesh_constants.cameraPtr    = m_cameraUBOs->getDeviceAddress();
				meshlet_mesh_constants.sceneDataPtr = m_sceneDataUBOs->getDeviceAddress();

				meshlet_mesh_constants.samplerIndex              = m_renderCtx->getSampler(render::ESamplerType::eIrradianceMap);
				meshlet_mesh_constants.diffuseIrradianceMapIndex = m_diffuseIrradianceMap->getAlignedShaderReadHeapID();

				vk_cmd.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(), 0u, vk::IndexType::eUint32);

				for (const auto &submesh: mesh->getMeshData().submeshes)
				{
					Dx::XMStoreFloat4x4(&meshlet_mesh_constants.meshTransform, Dx::XMMatrixMultiply(Dx::XMLoadFloat4x4(&submesh.transform), transform));

					meshlet_mesh_constants.materialIndex = submesh.materialIndex;
					cmd->pushData(meshlet_mesh_constants);

					cmd->drawIndexed(submesh.indexCount, 1, submesh.indexOffset, static_cast<int32>(submesh.vertexOffset), 0);
				}
			}
		}

		m_renderCtx->endRendering(rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;

		m_MSAAColour->resize(m_viewportSize);
		m_MSAADepth->resize(m_viewportSize);

		m_camera.onResize(p_size);
		m_skyboxPass->onResize(p_size);
	}

	auto onEvent(Event &p_event) -> void override
	{
		m_camera.onEvent(p_event);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	FPCamera            m_camera{};
	gpu::RawImageUnique m_MSAAColour{nullptr};
	gpu::RawImageUnique m_MSAADepth{nullptr};

	render::ImageHandle m_image{nullptr};

	UniquePtr<Scene> m_scene{nullptr};

	render::UniformBufferPFFUnique m_cameraUBOs{nullptr};
	render::UniformBufferPFFUnique m_sceneDataUBOs{nullptr};

	render::ImageHandle m_environmentMap{nullptr};
	render::ImageHandle m_diffuseIrradianceMap{nullptr};

	render::GraphicsStateUnique m_geometryGraphicsState{nullptr};

	render::DynamicMaterialUnique m_material{nullptr};

	TST_PUSH_CONSTANT_BLOCK(MeshletMeshConstants)
	{
		uintptr vertexBuffer;

		uintptr materialTest;

		Dx::XMFLOAT4X4 meshTransform;
		uintptr        materialBuffer;
		uint32         materialIndex;
		float32        _padd2[1];

		uintptr cameraPtr;
		uintptr sceneDataPtr;

		uint32 samplerIndex;
		uint32 diffuseIrradianceMapIndex;
	};

	UniquePtr<render::SkyboxPass> m_skyboxPass{nullptr};
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = true;
	app_spec.windowSpecInfo.startMaximized = true;
	Application app{app_spec, nullptr};

	app.addLayer<ClientLayer>();

	try
	{
		app.run();
	}
	catch (const vk::DeviceLostError &e)
	{
		LOG_FATAL("Device lost error: {}", e.what());

		return -1;
	}

	return 0;
}
