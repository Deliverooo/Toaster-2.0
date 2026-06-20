#include <toast_kernel/application.hpp>
#include <toast_kernel/fp_camera.hpp>
#include <toast_kernel/layer.hpp>

#include <toast_render/globals.hpp>
#include <toast_render/render_context.hpp>

#include <toast_lib/os/terminal.hpp>

#include "toast_render/dynamic_mesh.hpp"
#include "toast_render/storage_buffer.hpp"

using namespace toaster;

struct Mesh
{
	Mesh(render::RenderContext &p_render_ctx, const io::filesystem::Path &p_path) : m_renderCtx(&p_render_ctx)
	{
		meshData = render::importMeshFromFile(p_path);

		vertexBufferSSBO               = m_renderCtx->createUnique<render::StorageBuffer>(meshData.vertices, "Raw_mesh_vertices");
		meshletBufferSSBO              = m_renderCtx->createUnique<render::StorageBuffer>(meshData.meshlets, "Meshlets");
		meshletVertexIndexBufferSSBO   = m_renderCtx->createUnique<render::StorageBuffer>(meshData.meshletVertices, "Meshlet_vertices");
		meshletTriangleIndexBufferSSBO = m_renderCtx->createUnique<render::StorageBuffer>(meshData.meshletTriangles, "Meshlet_triangles");

		submeshBufferSSBO = m_renderCtx->createUnique<render::StorageBuffer>(meshData.submeshes, "Submeshes");
	}

	NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

	render::StorageBufferUnique vertexBufferSSBO{nullptr};
	render::StorageBufferUnique meshletBufferSSBO{nullptr};
	render::StorageBufferUnique meshletVertexIndexBufferSSBO{nullptr};
	render::StorageBufferUnique meshletTriangleIndexBufferSSBO{nullptr};
	render::StorageBufferUnique submeshBufferSSBO{nullptr};

	render::DynamicMeshData meshData;
};

class ClientLayer : public IAppLayer
{
public:
	struct SceneDataUB
	{
		Dx::XMFLOAT4 cameraPos;
	};

	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		auto binary_dir{os::getBinaryDirectory()};
		auto resources_dir{binary_dir / "../resources"};

		m_image = m_renderCtx->createImageRef(resources_dir / "textures/Peeber.png");

		m_geometryGraphicsState = m_renderCtx->createUnique<render::GraphicsState>();

		auto mesh_shader{m_globals->getShader("Dynamic_Mesh_MS")};
		auto pixel_shader{m_globals->getShader("Dynamic_Mesh_PS")};
		m_geometryGraphicsState->setShaders({mesh_shader, pixel_shader}).setAttachmentCount(1u).setCullMode(vk::CullModeFlagBits::eNone).setEnableDepthTest(true).
				setEnableDepthWrite(true);

		m_cameraUBOs    = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(render::Globals::ViewProjCameraUB));
		m_sceneDataUBOs = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(SceneDataUB));

		m_environmentMap       = m_renderCtx->createEnvironmentMapImage(resources_dir / "environments/overcast_soil_puresky_2k.hdr");
		m_diffuseIrradianceMap = m_renderCtx->createDiffuseIrradianceMapImage(m_environmentMap);

		m_mesh = m_renderCtx->createUnique<Mesh>(resources_dir / "meshes/source/Backrooms_Bake/Backrooms_Bake.obj");
	}

	auto onDestroy() -> void override
	{
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		const auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo(false, {0.025f, 0.025f, 0.025f, 1.0f}, true)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &     vk_cmd{cmd->getVulkanCommandBuffer()};

		m_geometryGraphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		m_camera.onUpdate(p_dt);

		SceneDataUB scene_data_ub{};
		Dx::XMStoreFloat4(&scene_data_ub.cameraPos, m_camera.getPosition());
		m_sceneDataUBOs->setData(scene_data_ub);

		render::Globals::ViewProjCameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, m_camera.getViewMatrix());
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, m_camera.getProjectionMatrix());
		m_cameraUBOs->setData(camera_ub);

		MeshletMeshConstants meshlet_mesh_constants{};
		meshlet_mesh_constants.vertexBuffer               = m_mesh->vertexBufferSSBO->getDeviceAddress();
		meshlet_mesh_constants.meshletBuffer              = m_mesh->meshletBufferSSBO->getDeviceAddress();
		meshlet_mesh_constants.meshletVertexIndexBuffer   = m_mesh->meshletVertexIndexBufferSSBO->getDeviceAddress();
		meshlet_mesh_constants.meshletTriangleIndexBuffer = m_mesh->meshletTriangleIndexBufferSSBO->getDeviceAddress();
		meshlet_mesh_constants.submeshBuffer              = m_mesh->submeshBufferSSBO->getDeviceAddress();

		meshlet_mesh_constants.cameraPtr    = m_cameraUBOs->getDeviceAddress();
		meshlet_mesh_constants.sceneDataPtr = m_sceneDataUBOs->getDeviceAddress();

		meshlet_mesh_constants.samplerIndex              = m_renderCtx->getSampler(render::ESamplerType::eIrradianceMap);
		meshlet_mesh_constants.diffuseIrradianceMapIndex = m_diffuseIrradianceMap->getAlignedShaderReadHeapID();
		cmd->pushData(meshlet_mesh_constants);

		vk_cmd.drawMeshTasksEXT(m_mesh->meshData.meshlets.size(), 1, 1);

		m_renderCtx->endRendering(rendering_info);
	}

	auto onResize(tsm::uint2 p_size) -> void override
	{
		m_viewportSize = p_size;
		m_camera.onResize(p_size);
	}

	auto onEvent(Event &p_event) -> void override
	{
		m_camera.onEvent(p_event);
	}

private:
	tsm::uint2 m_viewportSize{0u};

	FPCamera            m_camera{};
	render::ImageHandle m_image{nullptr};

	render::UniformBufferPFFUnique m_cameraUBOs{nullptr};
	render::UniformBufferPFFUnique m_sceneDataUBOs{nullptr};

	render::ImageHandle m_environmentMap{nullptr};
	render::ImageHandle m_diffuseIrradianceMap{nullptr};

	render::GraphicsStateUnique m_geometryGraphicsState{nullptr};

	UniquePtr<Mesh> m_mesh{nullptr};

	TST_PUSH_CONSTANT_BLOCK(MeshletMeshConstants)
	{
		uintptr vertexBuffer;
		uintptr meshletBuffer;
		uintptr meshletVertexIndexBuffer;
		uintptr meshletTriangleIndexBuffer;
		uintptr submeshBuffer;

		uintptr cameraPtr;
		uintptr sceneDataPtr;

		uint32 samplerIndex;
		uint32 diffuseIrradianceMapIndex;
	};
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
