#include <toast_kernel/application.hpp>
#include <toast_kernel/fp_camera.hpp>
#include <toast_kernel/layer.hpp>

#include <toast_render/globals.hpp>
#include <toast_render/render_context.hpp>

#include <toast_lib/os/terminal.hpp>

#include "toast_render/dynamic_mesh.hpp"
#include "toast_render/storage_buffer.hpp"

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		auto binary_dir{os::getBinaryDirectory()};
		auto resources_dir{binary_dir / "../resources"};

		m_image = m_renderCtx->createImageRef(resources_dir / "textures/Peeber.png");

		m_graphicsState = m_renderCtx->createUnique<render::GraphicsState>();

		auto mesh_shader{m_globals->getShader("Dynamic_Mesh_MS")};
		auto pixel_shader{m_globals->getShader("Dynamic_Mesh_PS")};
		m_graphicsState->setShaders({mesh_shader, pixel_shader}).setVertexBufferLayout(render::RenderContext::meshVbl).setAttachmentCount(1u).
				setCullMode(vk::CullModeFlagBits::eNone).setEnableDepthTest(false).setEnableDepthWrite(false);

		m_cameraUBOs = m_renderCtx->createUnique<render::UniformBufferPFF>(sizeof(render::Globals::ViewProjCameraUB));

		m_meshData = render::importMeshFromFile(resources_dir / "meshes/utah_teapot.obj");

		m_vertexBuffer               = m_renderCtx->createUnique<render::StorageBuffer>(m_meshData.vertices, "Raw_mesh_vertices");
		m_meshletBuffer              = m_renderCtx->createUnique<render::StorageBuffer>(m_meshData.meshlets, "Meshlets");
		m_meshletVertexIndexBuffer   = m_renderCtx->createUnique<render::StorageBuffer>(m_meshData.meshletVertices, "Meshlet_vertices");
		m_meshletTriangleIndexBuffer = m_renderCtx->createUnique<render::StorageBuffer>(m_meshData.meshletTriangles, "Meshlet_triangles");

		// vk::PhysicalDeviceMeshShaderPropertiesEXT mesh_shader_props{};
		// vk::PhysicalDeviceProperties2             props{};
		// props.pNext = &mesh_shader_props;
		// m_renderCtx->getPhysicalDevice()->getVulkanPhysicalDevice().getProperties2(&props);
		//
		// LOG_INFO("Max output vertices: {}", mesh_shader_props.maxMeshOutputVertices);
		// LOG_INFO("Max output primitives: {}", mesh_shader_props.maxMeshOutputPrimitives);
	}

	auto onDestroy() -> void override
	{
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		const auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo(false, {0.0f, 1.0f, 1.0f, 1.0f}, false)};
		const auto cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &     vk_cmd{cmd->getVulkanCommandBuffer()};

		m_graphicsState->bind();
		m_renderCtx->beginRendering(rendering_info);

		m_camera.onUpdate(p_dt);

		render::Globals::ViewProjCameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.viewMatrix, m_camera.getViewMatrix());
		Dx::XMStoreFloat4x4(&camera_ub.projectionMatrix, m_camera.getProjectionMatrix());
		m_cameraUBOs->setData(camera_ub);

		MeshletMeshConstants meshlet_mesh_constants{};
		meshlet_mesh_constants.vertexBuffer               = m_vertexBuffer->getDeviceAddress();
		meshlet_mesh_constants.meshletBuffer              = m_meshletBuffer->getDeviceAddress();
		meshlet_mesh_constants.meshletVertexIndexBuffer   = m_meshletVertexIndexBuffer->getDeviceAddress();
		meshlet_mesh_constants.meshletTriangleIndexBuffer = m_meshletTriangleIndexBuffer->getDeviceAddress();

		meshlet_mesh_constants.cameraPtr = m_cameraUBOs->getDeviceAddress();

		meshlet_mesh_constants.samplerIndex = m_renderCtx->getSampler(render::ESamplerType::eDefault);
		meshlet_mesh_constants.textureIndex = m_image->getAlignedShaderReadHeapID();

		cmd->pushData(meshlet_mesh_constants);

		vk_cmd.drawMeshTasksEXT(m_meshData.meshlets.size(), 1, 1);

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

	UniquePtr<render::GraphicsState> m_graphicsState{nullptr};
	render::ImageHandle              m_image{nullptr};

	FPCamera m_camera{};

	render::UniformBufferPFFUnique m_cameraUBOs{nullptr};

	render::StorageBufferUnique m_vertexBuffer{nullptr};
	render::StorageBufferUnique m_meshletBuffer{nullptr};
	render::StorageBufferUnique m_meshletVertexIndexBuffer{nullptr};
	render::StorageBufferUnique m_meshletTriangleIndexBuffer{nullptr};

	render::DynamicMeshData m_meshData;

	TST_PUSH_CONSTANT_BLOCK(MeshletMeshConstants)
	{
		uintptr vertexBuffer;
		uintptr meshletBuffer;
		uintptr meshletVertexIndexBuffer;
		uintptr meshletTriangleIndexBuffer;

		uintptr cameraPtr;

		uint32 samplerIndex;
		uint32 textureIndex;
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
