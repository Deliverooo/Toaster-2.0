#include <iostream>

#include "client_application.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_compute_pipeline.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_render/renderer.hpp"

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

#include "../../editor/source/editor_camera.hpp"
#include "GLFW/glfw3.h"
#include "toast_gpu/vk/vk_swapchain.hpp"
#include "toast_kernel/input.hpp"
#include "toast_lib/events/window_event.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer_2d.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
auto main(int32 p_argc, char **p_argv) -> int32
{
	#endif

	#pragma region create vulkan devices
	toaster::gpu::VKInstanceSpecInfo vk_instance_spec_info{};
	vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan";
	vk_instance_spec_info.requiredExtensions = toaster::Window::getRequiredInstanceExtensions();
	auto vk_instance{new toaster::gpu::VKInstance{vk_instance_spec_info}};

	std::unordered_set<toaster::String> required_device_extensions{
		vk::KHRSwapchainExtensionName,
		vk::KHRDynamicRenderingExtensionName,
		vk::KHRTimelineSemaphoreExtensionName,
		vk::EXTCustomBorderColorExtensionName,
		vk::KHRMaintenance6ExtensionName,
		vk::KHRLoadStoreOpNoneExtensionName
	};

	toaster::gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
	vk_physical_device_spec_info.requiredExtensions = required_device_extensions;

	auto vk_physical_device{new toaster::gpu::VKPhysicalDevice{vk_instance, vk_physical_device_spec_info}};

	toaster::gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
	vk_logical_device_spec_info.usePresent           = true;
	vk_logical_device_spec_info.printShaderDebugInfo = false;
	vk_logical_device_spec_info.requiredExtensions   = required_device_extensions;
	auto features{toaster::gpu::VKLogicalDeviceSpecInfo::getDefaultFeatures()};
	vk_logical_device_spec_info.pNext = features.get<vk::PhysicalDeviceFeatures2>();

	auto vk_logical_device{new toaster::gpu::VKLogicalDevice{vk_physical_device, vk_logical_device_spec_info}};
	#pragma endregion

	#pragma region create window
	toaster::Window::initWindowingAPI();
	toaster::WindowCreateInfo window_create_info{};
	window_create_info.width  = 1920;
	window_create_info.height = 1080;
	window_create_info.title  = "Toaster-2.0 -> Vulkan";
	auto window{new toaster::Window{vk_logical_device, window_create_info}};

	volatile bool window_closed{false};
	window->setEventCallback([&window_closed](toaster::Event &event) mutable -> void
	{
		toaster::EventDispatcher dispatcher{event};
		dispatcher.dispatch<toaster::WindowCloseEvent>([&window_closed](toaster::WindowCloseEvent &window_close_event) mutable -> bool
		{
			window_closed = true;
			return true;
		});
	});
	window->maximize();

	toaster::input::setCurrentWindowContext(window);
	#pragma endregion

	toaster::Globals::init(vk_logical_device);
	{
		auto   swapchain{window->getSwapchain()};
		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		auto                             fullscreen_shader{toaster::Globals::getShaderLibrary().get("Composite")};
		toaster::gpu::PipelineCreateInfo fullscreen_pipeline_create_info{};
		fullscreen_pipeline_create_info.colourAttachments  = {window->getSwapchain()->getSurfaceFormat().format};
		fullscreen_pipeline_create_info.depthFormat        = window->getSwapchain()->getDepthFormat();
		fullscreen_pipeline_create_info.shader             = fullscreen_shader;
		fullscreen_pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone; // We don't want to cull our viewport
		fullscreen_pipeline_create_info.vertexBufferLayout = toaster::gpu::BufferLayout{
			{toaster::gpu::EBufferDataType::eFloat3, "a_Position"},
			{toaster::gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};
		auto pipeline{vk_logical_device->alloc<toaster::gpu::VKPipeline>(fullscreen_pipeline_create_info)};
		auto render_pass{vk_logical_device->alloc<toaster::gpu::VKRenderPass>(pipeline)};
		render_pass->bake();

		toaster::Renderer2DSpecInfo renderer_2d_spec_info{};
		renderer_2d_spec_info.renderTargetWidth  = window_width;
		renderer_2d_spec_info.renderTargetHeight = window_height;
		auto renderer_2d{toaster::make_reference<toaster::Renderer2D>(vk_logical_device, renderer_2d_spec_info)};

		toaster::EditorCamera camera{90.0f, static_cast<float32>(window_width) / static_cast<float32>(window_height), 0.1f, 1000.0f};

		swapchain->setResizeCallback([&](const uint32 width, const uint32 height) -> void
		{
			window_width  = width;
			window_height = height;

			renderer_2d->onResize(width, height);
			camera.setViewportSize(static_cast<float32>(width), static_cast<float32>(height));
		});

		float32 lastFrameTime{0.0f};
		float32 deltaTime{0.0f};
		while (!window_closed)
		{
			const auto startTime{static_cast<float32>(glfwGetTime())};
			deltaTime     = startTime - lastFrameTime;
			lastFrameTime = startTime;

			window->processEvents();
			window->beginFrame();

			auto & command_buffer{swapchain->getCurrentCommandBuffer()};
			uint32 frame_index{swapchain->getFrameIndex()};

			camera.onUpdate(deltaTime);

			renderer_2d->begin(command_buffer, frame_index, camera.getViewMatrix(), camera.getProjectionMatrix());
			renderer_2d->submitQuad({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f});
			renderer_2d->end(command_buffer, frame_index);

			render_pass->setInput("u_Texture", renderer_2d->getColourOutput());

			toaster::gpu::RenderingInfo rendering_info{};
			rendering_info.renderArea = vk::Rect2D{{0, 0}, {window_width, window_height}};

			auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
			colour_attachment_info.imageView   = swapchain->getCurrentImageView();
			colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
			colour_attachment_info.clearValue  = vk::ClearColorValue{0.0f, 1.0f, 1.0f, 1.0f};

			toaster::gpu::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.imageView   = swapchain->getDepthImageView();
			depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.clearValue  = vk::ClearDepthStencilValue{1.0f, 0u};
			rendering_info.pDepthAttachment   = &depth_attachment_info;

			toaster::Renderer::beginRendering(rendering_info, command_buffer, frame_index, render_pass);
			toaster::Renderer::renderFullscreenQuad(command_buffer, frame_index, pipeline, nullptr);
			toaster::Renderer::endRendering(rendering_info, command_buffer);

			window->endFrame();
		}
	}

	vk_logical_device->getVulkanLogicalDevice().waitIdle();

	toaster::Globals::shutdown();
	vk_logical_device->performGarbageCollection();

	delete window;
	toaster::Window::shutdownWindowingAPI();
	toaster::input::setCurrentWindowContext(nullptr);

	delete vk_logical_device;
	delete vk_physical_device;
	delete vk_instance;

	return 0;
}
