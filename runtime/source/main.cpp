#include <iostream>

#include "runtime_application.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_compute_pipeline.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_render/renderer.hpp"

#include <GLFW/glfw3.h>
#include "fp_camera.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"
#include "toast_kernel/input.hpp"
#include "toast_lib/events/window_event.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer_2d.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/scene.hpp"
#include "toast_scene/scene_renderer.hpp"

#include "toast_lib/events/key_event.hpp"
#include "toast_lib/os/file_dialog.hpp"
#include "toast_lib/os/library_loading.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "toast_scripting/script_common.hpp"
#include "toast_scripting/script_engine.hpp"
#include "toast_scripting/script_object.hpp"

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
auto main(int32 p_argc, char **p_argv) -> int32
{
	#endif

	#ifndef TST_RUNTIME_DEMO
	toaster::ApplicationCreateInfo app_create_info{};
	app_create_info.windowCreateInfo.width          = 1920;
	app_create_info.windowCreateInfo.height         = 1080;
	app_create_info.windowCreateInfo.title          = "Toaster Vπ - Runtime;
	app_create_info.windowCreateInfo.iconPath       = "../resources/textures/OrboCloseup.png";
	app_create_info.windowCreateInfo.startMaximized = true;

	auto *app = new toaster::RuntimeApplication(app_create_info, p_argc, p_argv);

	app->run();
	delete app;
	return EXIT_SUCCESS;

	#else

	toaster::io::filesystem::Path executable_path{p_argv[0]};

	#pragma region create vulkan devices
	toaster::gpu::VKInstanceSpecInfo vk_instance_spec_info{}; vk_instance_spec_info.appName = "Toaster-2.0 -> Vulkan";
	vk_instance_spec_info.requiredExtensions = toaster::Window::getRequiredInstanceExtensions(); auto vk_instance{new toaster::gpu::VKInstance{vk_instance_spec_info}};
	std::unordered_set<toaster::String> required_device_extensions{
		vk::KHRSwapchainExtensionName,
		vk::KHRDynamicRenderingExtensionName,
		vk::KHRTimelineSemaphoreExtensionName,
		vk::EXTCustomBorderColorExtensionName,
		vk::KHRMaintenance6ExtensionName,
		vk::KHRLoadStoreOpNoneExtensionName
	}; toaster::gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{}; vk_physical_device_spec_info.requiredExtensions = required_device_extensions; auto
			vk_physical_device{new toaster::gpu::VKPhysicalDevice{vk_instance, vk_physical_device_spec_info}}; toaster::gpu::VKLogicalDeviceSpecInfo
			vk_logical_device_spec_info{}; vk_logical_device_spec_info.usePresent = true; vk_logical_device_spec_info.printShaderDebugInfo = false;
	vk_logical_device_spec_info.requiredExtensions = required_device_extensions; auto features{toaster::gpu::VKLogicalDeviceSpecInfo::getDefaultFeatures()};
	vk_logical_device_spec_info.pNext = features.get<vk::PhysicalDeviceFeatures2>(); auto vk_logical_device{
		new toaster::gpu::VKLogicalDevice{vk_physical_device, vk_logical_device_spec_info}
	};
	#pragma endregion

	#pragma region create window
	toaster::Window::initWindowingAPI(); toaster::WindowCreateInfo window_create_info{}; window_create_info.width = 1280u; window_create_info.height = 720u;
	window_create_info.title = "Toaster-2.0 -> Vulkan"; window_create_info.iconPath = "../resources/textures/WindowIcon001.png"; window_create_info.startMaximized = true;
	auto window{new toaster::Window{vk_logical_device, window_create_info}}; volatile bool window_closed{false}; window->setEventCallback([&window, &window_closed
																																		  ](toaster::Event &event) mutable
																																	  -> void
																																		  {
																																			  toaster::EventDispatcher
																																					  dispatcher{event};
																																			  dispatcher.dispatch<
																																				  toaster::WindowCloseEvent>([
																																												 &window_closed
																																											 ](
																																										 toaster::WindowCloseEvent
																																										 &window_close_event)
																																									 mutable
																																										 ->
																																										 bool
																																											 {
																																												 window_closed
																																														 = true;
																																												 return
																																														 true;
																																											 });
																																			  dispatcher.dispatch<
																																				  toaster::KeyPressEvent>([
																																											  &window
																																										  ](
																																									  toaster::KeyPressEvent
																																									  &key_press_event)
																																								  mutable
																																									  ->
																																									  bool
																																										  {
																																											  if
																																											  (key_press_event
																																											   .getKeyCode()
																																											   ==
																																											   toaster::input::EKeyCode::eF11)
																																											  {
																																												  if
																																												  (!
																																													  window
																																													  ->
																																													  isFullscreen())
																																													  window
																																															  ->
																																															  setFullscreen();
																																												  else
																																													  window
																																															  ->
																																															  setWindowed();
																																											  }

																																											  return
																																													  false;
																																										  });
																																		  }); static auto input_ctx{
		window->getInputContext()
	};
	#pragma endregion

	toaster::Globals::init(vk_logical_device); toaster::io::filesystem::Path scripts_dir{"../scripts"};

	#if 0
	constexpr bool force_compile_scripts{true}; if (!toaster::io::filesystem::exists(scripts_dir / "Toaster/bin") || force_compile_scripts)
	{
		toaster::io::filesystem::Path script_bat_path{"../scripts/build_scripts.bat"};
		toaster::String               build_scripts_cmd{std::format("cd ../scripts && {}", std::filesystem::absolute(script_bat_path).string())};
		std::system(build_scripts_cmd.c_str());
	}
	#endif

	toaster::script::ScriptEngineSpecInfo script_engine_spec_info{}; script_engine_spec_info.rootDomainName = "ToasterRootDomain";
	script_engine_spec_info.appDomainName = "ToasterAppDomain"; script_engine_spec_info.assemblyPath = scripts_dir / "Toaster/bin/Debug/net48/Toaster.dll";
	toaster::script::ScriptEngine script_engine{script_engine_spec_info};

	{
		static auto scene{new toaster::Scene{vk_logical_device, &script_engine, "Main Scene"}};

		script_engine.registerMethod("Toaster.Input::IsKeyDown", +[](toaster::input::EKeyCode p_key_code) -> bool
		{
			return input_ctx->isKeyDown(p_key_code);
		});

		script_engine.registerMethod("Toaster.Input::IsMouseButtonDown", +[](toaster::input::EMouseButton p_mouse_button) -> bool
		{
			return input_ctx->isMouseButtonDown(p_mouse_button);
		});

		script_engine.registerMethod("Toaster.Input::GetCursorMode", +[](toaster::input::ECursorMode *p_cursor_mode) -> void
		{
			*p_cursor_mode = input_ctx->getCursorMode();
		});

		script_engine.registerMethod("Toaster.Input::SetCursorMode", +[](toaster::input::ECursorMode p_cursor_mode) -> void
		{
			input_ctx->setCursorMode(p_cursor_mode);
		});

		script_engine.registerMethod("Toaster.Input::GetMousePos", +[](glm::vec2 *p_out_pos) -> void
		{
			auto [x, y]{input_ctx->getMousePos()};
			*p_out_pos = glm::vec2{x, y};
		});

		script_engine.registerMethod("Toaster.Orbo::NativeTest", +[]() -> void { LOG_INFO("Hello Native Test!"); });
		script_engine.registerMethod("Toaster.Orbo::NativeOrbo", +[]() -> void { LOG_INFO("Hello Native Orbo!"); });

		script_engine.registerMethod("Toaster.InternalCalls::HasComponent", +[](uint32 p_entity_id, MonoReflectionType *p_component_type) -> bool
		{
			MonoType *      type{mono_reflection_type_get_type(p_component_type)};
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			return scene->getHasComponentFn(type)(&entity);
		});

		script_engine.registerMethod("Toaster.InternalCalls::AddComponent", +[](uint32 p_entity_id, MonoReflectionType *p_component_type) -> void
		{
			MonoType *      type{mono_reflection_type_get_type(p_component_type)};
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			scene->getAddComponentFn(type)(&entity);
		});

		script_engine.registerMethod("Toaster.TagComponent::GetTag", +[](uint32 p_entity_id, MonoString **p_out_tag) -> void
		{
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			*p_out_tag = mono_string_new(scene->getScriptEngine()->getAppDomain(), entity.getComponent<toaster::TagComponent>().tag.c_str());
		});

		script_engine.registerMethod("Toaster.TagComponent::SetTag", +[](uint32 p_entity_id, MonoString **p_tag) -> void
		{
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};

			char *new_string{mono_string_to_utf8(*p_tag)};
			entity.getComponent<toaster::TagComponent>().tag = toaster::String{new_string};
			mono_free(new_string);
		});

		script_engine.registerMethod("Toaster.TransformComponent::GetTranslation", +[](uint32 p_entity_id, glm::vec3 *p_out_translation) -> void
		{
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			*p_out_translation = entity.getComponent<toaster::TransformComponent>().translation;
		});

		script_engine.registerMethod("Toaster.TransformComponent::SetTranslation", +[](uint32 p_entity_id, glm::vec3 *p_translation) -> void
		{
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			entity.getComponent<toaster::TransformComponent>().translation = *p_translation;
		});

		script_engine.registerMethod("Toaster.SpriteRendererComponent::GetColour", +[](uint32 p_entity_id, glm::vec4 *p_out_colour) -> void
		{
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			*p_out_colour = entity.getComponent<toaster::SpriteRendererComponent>().colour;
		});

		script_engine.registerMethod("Toaster.SpriteRendererComponent::SetColour", +[](uint32 p_entity_id, glm::vec4 *p_colour) -> void
		{
			toaster::Entity entity{static_cast<entt::entity>(p_entity_id), scene};
			entity.getComponent<toaster::SpriteRendererComponent>().colour = *p_colour;
		});

		toaster::io::filesystem::Path shader_dir{"../source/toaster/toast_shaders"};

		toaster::ShaderLibrary shader_lib{};
		shader_lib.add("Mesh Test", toaster::gpu::VKShaderCompiler::compileToShaderFromPaths(vk_logical_device,
																							 {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment}, {
																								 shader_dir / "mesh.vert.glsl",
																								 shader_dir / "mesh.pixel.glsl"
																							 }));

		toaster::gpu::TextureSpecInfo texture_spec_info{};
		texture_spec_info.width        = 1u;
		texture_spec_info.height       = 1u;
		texture_spec_info.format       = vk::Format::eR8G8B8A8Unorm;
		texture_spec_info.usage        = toaster::gpu::ETextureUsage::eShaderSampled;
		texture_spec_info.generateMips = false;
		auto         tex_2d{vk_logical_device->alloc<toaster::gpu::VKTexture2D>(texture_spec_info)};
		const uint64 tex_size{texture_spec_info.width * texture_spec_info.height * 4};
		uint32       tex_data{0xFFFFFFFF};
		tex_2d->setData(&tex_data, tex_size);
		tex_2d->createSampler();

		{
			toaster::Entity orbo_entity{scene->createEntity("Orbo")};
			orbo_entity.addComponent<toaster::MeshComponent>().mesh = vk_logical_device->alloc<toaster::gpu::VKMesh>("../resources/meshes/Orbo.fbx",
																													 shader_lib.get("Mesh Test"));

			orbo_entity.addComponent<toaster::ScriptComponent>().className = "Toaster.Player";
		}

		if constexpr (false)
		{
			toaster::Entity peeb_entity{scene->createEntity("Peeb")};
			auto &          src{peeb_entity.addComponent<toaster::SpriteRendererComponent>()};
			src.texture = tex_2d;

			auto &cam{peeb_entity.addComponent<toaster::CameraComponent>()};
			cam.primary = true;
			cam.camera.setProjectionType(toaster::SceneCamera::EProjectionType::ePerspective);

			auto &trans{peeb_entity.getComponent<toaster::TransformComponent>()};
			trans.translation.z = 10.0f;
		}

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

		toaster::SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth  = window_width;
		scene_renderer_spec_info.viewportHeight = window_height;
		scene_renderer_spec_info.scene          = scene;
		auto scene_renderer{toaster::make_reference<toaster::SceneRenderer>(vk_logical_device, scene_renderer_spec_info)};

		toaster::FPCamera camera{input_ctx, 90.0f, static_cast<float32>(window_width) / static_cast<float32>(window_height), 0.1f, 1000.0f};

		swapchain->setResizeCallback([&](const uint32 width, const uint32 height) -> void
		{
			window_width  = width;
			window_height = height;

			scene->setViewportSize(width, height);

			scene_renderer->onResize(width, height);
			camera.setViewportSize(static_cast<float32>(width), static_cast<float32>(height));
		});

		float32 last_frame_time{0.0f};
		float32 dt{0.0f};
		while (!window_closed)
		{
			const auto start_time{static_cast<float32>(glfwGetTime())};
			dt              = start_time - last_frame_time;
			last_frame_time = start_time;

			window->processEvents();
			window->beginFrame();

			auto & command_buffer{swapchain->getCurrentCommandBuffer()};
			uint32 frame_index{swapchain->getFrameIndex()};

			camera.onUpdate(dt);

			scene->onUpdate(dt);
			scene->onRender(command_buffer, frame_index, dt, scene_renderer, camera.getViewMatrix(), camera.getProjectionMatrix());
			// scene->onRender(command_buffer, frame_index, dt, scene_renderer);

			render_pass->setInput("u_Texture", scene_renderer->getOutputColourTexture());

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
			rendering_info.pDepthAttachment   = std::addressof(depth_attachment_info);

			toaster::render::beginRendering(rendering_info, command_buffer, frame_index, render_pass);
			toaster::render::renderFullscreenQuad(command_buffer, frame_index, pipeline, nullptr);
			toaster::render::endRendering(rendering_info, command_buffer);

			window->endFrame();
		}

		delete scene;
	} vk_logical_device->getVulkanLogicalDevice().waitIdle(); toaster::Globals::shutdown(); vk_logical_device->performGarbageCollection(); delete window;
	toaster::Window::shutdownWindowingAPI(); delete vk_logical_device; delete vk_physical_device; delete vk_instance; return 0;

	#endif
}
