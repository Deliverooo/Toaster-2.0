#include "toast_gpu/vk/vk_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_kernel/application.hpp"
#include "toast_kernel/input.hpp"
#include "toast_lib/os/file_dialog.hpp"
#include "toast_lib/os/terminal.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/graphics_state.hpp"
#include "toast_render/image.hpp"
#include "toast_render/render_context.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scene.hpp"
#include "toast_scene/scene_renderer.hpp"

#include "toast_kernel/fp_camera.hpp"
#include "toast_math/colours.hpp"

#include <stb/stb_image.h>

using namespace toaster;

class ClientLayer : public IAppLayer
{
public:
	struct CameraUB
	{
		Dx::XMFLOAT4X4 view;
		Dx::XMFLOAT4X4 proj;
		Dx::XMFLOAT4X4 invProj;
	};

	auto onInit() -> void override
	{
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		m_camera = FPCamera{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f};

		auto binary_dir{os::getBinaryDirectory()};
		auto resources_dir{binary_dir / "../resources"};
		// auto logical_device{m_renderCtx->getLogicalDevice()};

		m_ubo = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(tsm::float4));
		m_ubo->setAllData(tsm::float4{0.0f, 1.0f, 1.0f, 1.0f});

		m_cameraUBO        = m_renderCtx->createRef<render::UniformBufferPFF>(sizeof(CameraUB));
		m_mappedCameraUBOs = m_cameraUBO->mapAllMemory(sizeof(CameraUB));

		render::ImageSpecInfo image_spec{};
		image_spec.format = vk::Format::eR8G8B8A8Unorm;
		image_spec.size   = {2u};

		{
			Buffer image_data;
			image_data.allocate(sizeof(uint32) * 4);
			image_data.writeType<uint32>(tsm::colours::rgbaToHex(tsm::colours::weezer), 0);
			image_data.writeType<uint32>(tsm::colours::rgbaToHex(tsm::colours::magenta), sizeof(uint32));
			image_data.writeType<uint32>(tsm::colours::rgbaToHex(tsm::colours::blue), sizeof(uint32) * 2);
			image_data.writeType<uint32>(tsm::colours::rgbaToHex(tsm::colours::red), sizeof(uint32) * 3);
			m_image = m_renderCtx->createRef<render::Image>(image_spec, image_data);
			image_data.release();
		}

		m_image = m_renderCtx->createImageRef(resources_dir / "textures/Peeber.png");

		m_vertexShader = m_renderCtx->createShader(resources_dir / "shaders/mesh_dynamic.vert.hlsl", render::EShaderStage::eVertex, render::EShaderStage::ePixel,
												   render::EShaderLanguage::eHLSL);
		m_fragmentShader = m_renderCtx->createShader(resources_dir / "shaders/mesh_dynamic.pixel.glsl", render::EShaderStage::ePixel, render::EShaderStage::eNone,
													 render::EShaderLanguage::eGLSL);

		m_graphicsState = m_renderCtx->createUnique<render::GraphicsState>();
		m_graphicsState->setShaders({m_vertexShader, m_fragmentShader}).setVertexBufferLayout(render::RenderContext::meshVbl).setAttachmentCount(1u).
				setCullMode(vk::CullModeFlagBits::eNone);

		m_mesh = m_renderCtx->createRef<render::MeshData>(resources_dir / "meshes/DJT_sculpt.fbx");
	}

	auto onDestroy() -> void override
	{
		m_cameraUBO->unmapAllMemory();
	}

	auto onUpdate(float32 p_dt) -> void override
	{
		auto rendering_info{m_app->getWindow().getSwapchainRenderingInfo({0.2f, 1.0f, 1.0f, 1.0f}, false)};

		auto  cmd{m_renderCtx->getCurrentSwapchainCommandBuffer()};
		auto &vk_cmd{cmd->getVulkanCommandBuffer()};

		m_renderCtx->beginRendering(rendering_info);
		cmd->setRenderArea(rendering_info.renderArea);

		m_renderCtx->getDescriptorHeap()->bind();

		m_camera.onUpdate(p_dt);

		CameraUB camera_ub{};
		Dx::XMStoreFloat4x4(&camera_ub.view, m_camera.getViewMatrix());
		Dx::XMStoreFloat4x4(&camera_ub.proj, m_camera.getProjectionMatrix());
		Dx::XMStoreFloat4x4(&camera_ub.invProj, Dx::XMMatrixInverse(nullptr, m_camera.getProjectionMatrix()));

		std::memcpy(m_mappedCameraUBOs[m_renderCtx->getCurrentFrameIndex()], &camera_ub, sizeof(CameraUB));

		struct PushConstants
		{
			uint32 textureIndex;
			uint32 samplerIndex;

			uintptr cameraAddress;

			Dx::XMFLOAT4X4 model;
		};

		PushConstants pcs{};
		pcs.textureIndex  = m_image->getAlignedHeapID();
		pcs.samplerIndex  = m_renderCtx->getSampler(m_activeSampler);
		pcs.cameraAddress = m_cameraUBO->getDeviceAddress();
		pcs.model         = m_mesh->getSubmeshes()[0].transform;

		cmd->pushData(pcs);

		m_graphicsState->bind();

		m_mesh->getVertexBuffer()->bind();
		m_mesh->getIndexBuffer()->bind();

		// m_renderCtx->renderMesh(m_mesh, 0)

		cmd->drawIndexed(m_mesh->getIndices().size());

		vk_cmd.endRendering();
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

	gpu::DynamicShaderHandle m_vertexShader;
	gpu::DynamicShaderHandle m_fragmentShader;

	UniquePtr<render::GraphicsState> m_graphicsState{nullptr};

	render::ImageHandle            m_image{nullptr};
	render::UniformBufferPFFHandle m_ubo{nullptr};

	render::UniformBufferPFFHandle m_cameraUBO{nullptr};
	std::vector<void *>            m_mappedCameraUBOs;

	FPCamera m_camera{};

	render::ESamplerType m_activeSampler{render::ESamplerType::eDefault};

	render::MeshHandle m_mesh{nullptr};

	// UniquePtr<render::Renderer2DV2> m_renderer2D{nullptr};
};

auto main(int32 p_argc, char **p_argv) -> int32
{
	ApplicationSpecInfo app_spec{};
	app_spec.printGPUDebugInfo             = true;
	app_spec.windowSpecInfo.startMaximized = true;
	Application app{app_spec, nullptr};

	app.addLayer<ClientLayer>();

	app.run();
	return 0;
}
