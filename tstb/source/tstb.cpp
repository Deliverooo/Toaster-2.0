#include "tstb.hpp"

#include "toast_lib/logging.hpp"
#include "toast_lib/os/terminal.hpp"

namespace toaster::tstb
{
	constexpr auto c_dotnetFrameworkVersion{"net48"};
	constexpr auto c_dotnetLanguageVersion{"10.0"};
	constexpr auto c_dotnetProfile{"Debug"};

	constexpr auto c_CMakeVersion{"3.30"};
	constexpr auto c_cppStandard{"23"};

	constexpr auto c_csprojTemplate{
		R"(<Project Sdk="Microsoft.NET.Sdk">
		<PropertyGroup>
			<TargetFramework>{0}</TargetFramework>
			<LangVersion>{1}</LangVersion>
			<ImplicitUsings>enable</ImplicitUsings>
			<Nullable>enable</Nullable>
		</PropertyGroup>
	<ItemGroup>
	  <Reference Include="Toaster">
	    <HintPath>{2}</HintPath>
	  </Reference>
	</ItemGroup>
	</Project>)"
	};

	constexpr auto c_gitignoreData{
		R"(.vs/
.idea/
.vscode/
resources/scripts/bin/
resources/scripts/obj/
	)"
	};

	constexpr auto c_RootCMakeListsTxtData{
		R"(cmake_minimum_required(VERSION {0})

set(CMAKE_CXX_STANDARD {1})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)

if (MSVC)
	add_compile_options(/MP)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${{CMAKE_SOURCE_DIR}}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${{CMAKE_SOURCE_DIR}}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${{CMAKE_SOURCE_DIR}}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${{CMAKE_RUNTIME_OUTPUT_DIRECTORY}}")
# Prevent in-source builds
if (${{CMAKE_SOURCE_DIR}} STREQUAL ${{CMAKE_BINARY_DIR}})
	message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif ()

# Project definition
project("{2}")

find_package(Toaster REQUIRED)

# Select C++23 as the standard for C++ projects.
# If C++23 is not available, downgrading to an earlier standard is NOT OK.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler specific language extensions.
set(CMAKE_CXX_EXTENSIONS OFF)

# Exe setup
add_executable("{2}")

# Make sure that no command line shows up in release mode
set_target_properties("{2}" PROPERTIES
		WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>"
)

set(SOURCE_FILES
		src/new_layer.cpp
		src/main.cpp
)

target_sources("{2}" PRIVATE ${{SOURCE_FILES}})

target_include_directories("{2}" PRIVATE "${{CMAKE_CURRENT_SOURCE_DIR}}/include")

target_link_libraries("{2}" PRIVATE Toaster::ToastKernel)
)"
	};

	constexpr auto c_mainCppData{
		R"(
#include "{0}/new_layer.hpp"

#ifndef NDEBUG
auto main(int32 p_argc, char **p_argv) -> int32
{{
#else
#ifndef _WINDOWS_
#include <Windows.h>
#undef min // Why windows? :(
#undef max
#endif
INT APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{{
	#endif

	char *toaster_sdk_dir{{std::getenv("TOASTER_SDK")}};

	if (!toaster_sdk_dir)
		TST_PERMA_ASSERT_MSG(false, "Dondé está Toaster? How did you manage to build ts without the SDK");

	tst::ApplicationSpecInfo app_spec{{}};
	app_spec.printGPUDebugInfo             = false;
	app_spec.windowSpecInfo.title          = "{0}";
	app_spec.windowSpecInfo.startMaximized = true;
	tst::Application app{{app_spec, nullptr}};

	app.addLayer<{0}::NewLayer>();

	// Run the app!!!
	app.run();

	return 0;
}}
)"
	};

	constexpr auto c_newLayerHeaderData{
		R"(
#pragma once

#include <toast_kernel/application.hpp>

#include <toast_scene/entity.hpp>
#include <toast_scene/scene_renderer.hpp>

namespace tst = toaster;

namespace {0}
{{
	class NewLayer : public tst::IAppLayer
	{{
	public:
		auto onInit() -> void override;
		auto onDestroy() -> void override;
		auto onUpdate(float32 p_dt) -> void override;
		auto onEvent(tst::Event &p_event) -> void override;
		auto onResize(tsm::uint2 p_size) -> void override;

	private:
		// It is good practice to store the viewport's current size
		tsm::uint2 m_viewportSize{{0u}};

		// Ts is needed so we can render to the swapchain... :)
		tst::render::RenderPassHandle m_swapchainPass{{nullptr}};

		tst::UniquePtr<tst::Scene>         m_scene{{nullptr}};
		tst::UniquePtr<tst::SceneRenderer> m_sceneRenderer{{nullptr}};

		tst::Entity m_cameraEntity;
		tst::Entity m_quadEntity;
	}};
}}
)"
	};

	constexpr auto c_newLayerCppData{
		R"(
#include "{0}/new_layer.hpp"

#include <toast_kernel/fp_camera.hpp>

#include <toast_math/colours.hpp>

#include <toast_lib/os/terminal.hpp>

#include <toast_render/globals.hpp>
#include <toast_render/render_context.hpp>

namespace {0}
{{
	auto NewLayer::onInit() -> void
	{{
		// Get the initial viewport size
		m_viewportSize = m_app->getWindow().getRenderAreaSize();

		// Create both the scene and scene renderer
		m_scene         = m_app->createScene("New_Scene");
		m_sceneRenderer = m_app->createSceneRenderer(m_scene.get());

		// Boilerplate swapchain rendering setup
		tst::gpu::PipelineSpecInfo pipeline_spec_info{{}};
		pipeline_spec_info.colourAttachments = {{vk::Format::eR8G8B8A8Srgb}};

		// You can use any shader you want E.g. post-processing, I am just using the most basic one... :)
		pipeline_spec_info.shader             = m_globals->shaderLibrary().get("Composite");
		pipeline_spec_info.vertexBufferLayout = tst::render::RenderContext::fullscreenQuadVbl;
		auto pipeline{{m_renderCtx->createGPURef<tst::gpu::Pipeline>(pipeline_spec_info, "Composite")}};

		// Create the swapchain pass. The texture is what the final scene will look like... :)
		m_swapchainPass = m_renderCtx->createRef<tst::render::RenderPass>(pipeline);
		m_swapchainPass->setInput("u_Texture", m_sceneRenderer->getColourTexture()).bake();

		// Set the scene environment to a skybox thingy
		tst::io::filesystem::Path environment_map_path{{tst::os::getBinaryDirectory() / "../resources/environments/Toaster-Default.hdr"}};
		m_scene->setSceneEnvironment(m_renderCtx->createEnvironmentMap(environment_map_path));

		// Create the main camera entity
		m_cameraEntity = m_scene->createEntity("Main_Camera");
		tst::FirstPersonCameraEntityCreateParams params{{m_inputCtx, 90.0f, m_viewportSize.aspect(), 0.1f, 1000.0f}};
		m_cameraEntity.addComponent<tst::NativeScriptComponent>().bind<tst::FirstPersonCameraEntity>(&params);
		m_scene->initNativeScripts(); // I have to call ts

		// Example of how to create an entity and add a sprite renderer component
		m_quadEntity = m_scene->createEntity("Quad");
		auto &sprite_renderer{{m_quadEntity.addComponent<tst::SpriteRendererComponent>()}};
		sprite_renderer.colour = tsm::colours::weezer; // Use tsm::colours for easy colour templates, such as the elusive weezer colour... :}}
	}}

	auto NewLayer::onDestroy() -> void
	{{
		// Trigger layer-specific destruction here
	}}

	auto NewLayer::onUpdate(float32 p_dt) -> void
	{{
		// Update and render the scene
		m_scene->onUpdate(p_dt);
		m_sceneRenderer->onRender();

		// Get the swapchain's rendering info from the window and set the clear colour to white, while using no depth because the scene renderer provides that.
		const auto rendering_info{{m_app->getWindow().getSwapchainRenderingInfo({{1.0f, 1.0f, 1.0f, 1.0f}}, false)}};
		const auto cmd{{m_renderCtx->getCurrentSwapchainCommandBuffer()}};

		// Render a fullscreen quad with the scene renderer's output as the texture
		m_renderCtx->beginRendering(cmd, rendering_info, m_swapchainPass);
		m_renderCtx->renderFullscreenQuad(cmd, m_swapchainPass, nullptr);
		m_renderCtx->endRendering(cmd, rendering_info);
	}}

	auto NewLayer::onEvent(tst::Event &p_event) -> void
	{{
		// Handle events here
		m_scene->onEvent(p_event);
	}}

	auto NewLayer::onResize(tsm::uint2 p_size) -> void
	{{
		m_viewportSize = p_size;

		m_scene->onResize(p_size);
		m_sceneRenderer->onResize(p_size);
	}}
}}
)"
	};

	constexpr auto c_buildProjectScriptData{
		R"(
mkdir build
cmake -B build
cmake --build build --config Release)"
	};

	TstB::TstB()
	{
	}

	auto TstB::newCppProj(const argparse::ArgumentParser &p_new_cpp_proj_cmd) -> int32
	{
		auto project_name{p_new_cpp_proj_cmd.get<String>("--name")};
		LOG_INFO("Creating new C++ project: {}", project_name);

		#pragma region create directories
		#define CREATE_DIRECTORY(__path, __info)\
		LOG_INFO("Creating {} directory", #__info); do { if (!std::filesystem::create_directory(__path)) {\
		LOG_ERROR("Directory already exists or creation failed"); return -1; } } while(false)

		const io::filesystem::Path project_root{project_name};
		CREATE_DIRECTORY(project_root, project root);

		const io::filesystem::Path source_dir{project_root / "src"};
		// Where the main file and the actual c++ code is
		CREATE_DIRECTORY(source_dir, src);

		const io::filesystem::Path include_dir{project_root / "include"};
		CREATE_DIRECTORY(include_dir, include);
		CREATE_DIRECTORY(include_dir / project_name, actual include directory);

		// It is useful to have resources
		const io::filesystem::Path resource_directory{project_root / "resources"};
		CREATE_DIRECTORY(resource_directory, resources);
		CREATE_DIRECTORY(resource_directory / "scenes", scenes);
		CREATE_DIRECTORY(resource_directory / "meshes", meshes);
		CREATE_DIRECTORY(resource_directory / "textures", textures);
		CREATE_DIRECTORY(resource_directory / "environments", environments);

		#undef CREATE_DIRECTORY
		#pragma endregion

		LOG_INFO("Creating CMakeLists.txt(s)");

		{
			const String root_cmake_file_data{fmt::format(c_RootCMakeListsTxtData, c_CMakeVersion, c_cppStandard, project_name)};
			io::filesystem::writeFile(project_root / "CMakeLists.txt", root_cmake_file_data);
		}

		LOG_INFO("Creating main.cpp");
		{
			const String main_cpp_file_data{fmt::format(c_mainCppData, project_name)};
			io::filesystem::writeFile(source_dir / "main.cpp", main_cpp_file_data);
		}

		LOG_INFO("Creating new_layer.hpp / new_layer.cpp");
		{
			const String new_layer_hpp_file_data{fmt::format(c_newLayerHeaderData, project_name)};
			io::filesystem::writeFile(include_dir / project_name / "new_layer.hpp", new_layer_hpp_file_data);

			const String new_layer_cpp_file_data{fmt::format(c_newLayerCppData, project_name)};
			io::filesystem::writeFile(source_dir / "new_layer.cpp", new_layer_cpp_file_data);
		}

		LOG_INFO("Creating .gitignore");
		io::filesystem::writeFile(project_root / ".gitignore", c_gitignoreData);

		LOG_INFO("Creating utility build script");
		io::filesystem::writeFile(project_root / "build_project.bat", c_buildProjectScriptData);

		LOG_INFO("Adding default resources");
		std::filesystem::copy_file(os::getBinaryDirectory() / "../resources/environments/overcast_soil_puresky_2k.hdr",
								   fmt::format("{0}/Toaster-Default.hdr", resource_directory / "environments"), std::filesystem::copy_options::overwrite_existing);

		return 0;
	}
}
