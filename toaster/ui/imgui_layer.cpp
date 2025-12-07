#include "imgui_layer.hpp"

#include <filesystem>

#include "platform/application.hpp"

#include "imgui_fonts.hpp"
#include "gpu/vulkan/vulkan_device_manager.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

namespace tst
{
	void ImGuiLayer::onInit()
	{
		std::filesystem::current_path(R"(C:\dev\Toaster\tools\sandbox)");

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		(void) io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

		// Configure Fonts
		{
			ui::FontConfiguration robotoBold;
			robotoBold.FontName = "Bold";
			robotoBold.FilePath = "res/fonts/roboto/Roboto-Bold.ttf";
			robotoBold.Size     = 18.0f;
			ui::Fonts::add(robotoBold);

			ui::FontConfiguration robotoLarge;
			robotoLarge.FontName = "Large";
			robotoLarge.FilePath = "res/fonts/roboto/Roboto-Regular.ttf";
			robotoLarge.Size     = 24.0f;
			ui::Fonts::add(robotoLarge);

			ui::FontConfiguration robotoDefault;
			robotoDefault.FontName = "Default";
			robotoDefault.FilePath = "res/fonts/roboto/Roboto-SemiMedium.ttf";
			robotoDefault.Size     = 15.0f;
			ui::Fonts::add(robotoDefault, true);

			static const ImWchar  s_FontAwesomeRanges[] = {0xf000, 0xf307, 0};
			ui::FontConfiguration fontAwesome;
			fontAwesome.FontName      = "FontAwesome";
			fontAwesome.FilePath      = "res/fonts/fontAwesome/fontawesome-webfont.ttf";
			fontAwesome.Size          = 16.0f;
			fontAwesome.GlyphRanges   = s_FontAwesomeRanges;
			fontAwesome.MergeWithLast = true;
			ui::Fonts::add(fontAwesome);

			ui::FontConfiguration robotoMedium;
			robotoMedium.FontName = "Medium";
			robotoMedium.FilePath = "res/fonts/roboto/Roboto-SemiMedium.ttf";
			robotoMedium.Size     = 18.0f;
			ui::Fonts::add(robotoMedium);

			ui::FontConfiguration robotoSmall;
			robotoSmall.FontName = "Small";
			robotoSmall.FilePath = "res/fonts/roboto/Roboto-SemiMedium.ttf";
			robotoSmall.Size     = 12.0f;
			ui::Fonts::add(robotoSmall);

			ui::FontConfiguration robotoExtraSmall;
			robotoExtraSmall.FontName = "ExtraSmall";
			robotoExtraSmall.FilePath = "res/fonts/roboto/Roboto-SemiMedium.ttf";
			robotoExtraSmall.Size     = 10.0f;
			ui::Fonts::add(robotoExtraSmall);

			ui::FontConfiguration robotoBoldTitle;
			robotoBoldTitle.FontName = "BoldTitle";
			robotoBoldTitle.FilePath = "res/fonts/roboto/Roboto-Bold.ttf";
			robotoBoldTitle.Size     = 16.0f;
			ui::Fonts::add(robotoBoldTitle);
		}

		ImGui::StyleColorsDark();
		setDarkThemeColors();

		ImGuiStyle &style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding              = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, style.Colors[ImGuiCol_WindowBg].w);

		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow *>(Application::getInstance().getMainWindow().getNativeWindow()), true);

		m_ImGuiRenderer = std::make_unique<ImGuiRenderer>();
		m_ImGuiRenderer->init();

		initPlatformInterface();
	}

	void ImGuiLayer::onDestroy()
	{
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::begin()
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

		m_ImGuiRenderer->updateFontTexture();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		// ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::end()
	{
		ImGui::Render();

		m_ImGuiRenderer->renderToSwapchain(ImGui::GetMainViewport(), Application::getInstance().getMainWindow().getSwapchain());

		// Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	struct ImGuiViewportData
	{
		bool                             WindowOwned = false;
		std::unique_ptr<VulkanSwapChain> SC;
		std::unique_ptr<ImGuiRenderer>   Renderer;
	};

	static void ImGuiRenderer_CreateWindow(ImGuiViewport *viewport)
	{
		ImGuiViewportData *data    = IM_NEW(ImGuiViewportData)();
		viewport->RendererUserData = data;

		vk::Instance vInstance = static_cast<VulkanDeviceManager *>(Application::getDeviceManager())->GetVulkanInstance();

		// Create surface
		ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
		vk::SurfaceKHR   surface;
		VkResult         err = static_cast<VkResult>(platform_io.Platform_CreateVkSurface(viewport, *(ImU64 *) &vInstance, nullptr, (ImU64 *) &surface));
		TST_ASSERT(err == VkResult::VK_SUCCESS);

		data->SC = std::make_unique<VulkanSwapChain>(surface);
		data->SC->create(static_cast<uint32>(viewport->Size.x), static_cast<uint32>(viewport->Size.y));
		data->WindowOwned = true;

		data->Renderer = std::make_unique<ImGuiRenderer>();
		data->Renderer->init();
	}

	static void ImGuiRenderer_DestroyWindow(ImGuiViewport *viewport)
	{
		ImGuiViewportData *vd = static_cast<ImGuiViewportData *>(viewport->RendererUserData);
		tdelete vd;
		viewport->RendererUserData = nullptr;
	}

	static void ImGuiRenderer_SetWindowSize(ImGuiViewport *viewport, ImVec2 size)
	{
		ImGuiViewportData *vd = static_cast<ImGuiViewportData *>(viewport->RendererUserData);
		vd->SC->onResize((uint32) size.x, (uint32) size.y);
	}

	static void ImGuiRenderer_RenderWindow(ImGuiViewport *viewport, void *)
	{
		ImGuiViewportData *vd = static_cast<ImGuiViewportData *>(viewport->RendererUserData);
		vd->SC->beginFrame();
		vd->Renderer->updateFontTexture();
		vd->Renderer->renderToSwapchain(viewport, vd->SC.get());
	}

	static void ImGuiRenderer_SwapBuffers(ImGuiViewport *viewport, void *)
	{
		ImGuiViewportData *vd = static_cast<ImGuiViewportData *>(viewport->RendererUserData);
		vd->SC->present();
	}

	void ImGuiLayer::initPlatformInterface()
	{
		ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			IM_ASSERT(platform_io.Platform_CreateVkSurface != nullptr && "Platform needs to setup the CreateVkSurface handler.");

		platform_io.Renderer_CreateWindow  = ImGuiRenderer_CreateWindow;
		platform_io.Renderer_DestroyWindow = ImGuiRenderer_DestroyWindow;
		platform_io.Renderer_SetWindowSize = ImGuiRenderer_SetWindowSize;
		platform_io.Renderer_RenderWindow  = ImGuiRenderer_RenderWindow;
		platform_io.Renderer_SwapBuffers   = ImGuiRenderer_SwapBuffers;
	}

	void ImGuiLayer::setDarkThemeColors()
	{
		auto &colors              = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

		// Headers
		colors[ImGuiCol_Header]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_HeaderActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Buttons
		colors[ImGuiCol_Button]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_ButtonActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Frame BG
		colors[ImGuiCol_FrameBg]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
		colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
		colors[ImGuiCol_FrameBgActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Tabs
		colors[ImGuiCol_Tab]                = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabHovered]         = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
		colors[ImGuiCol_TabActive]          = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
		colors[ImGuiCol_TabUnfocused]       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

		// Title
		colors[ImGuiCol_TitleBg]          = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgActive]    = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Resize Grip
		colors[ImGuiCol_ResizeGrip]        = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.0f);

		// Slider
		colors[ImGuiCol_SliderGrab]       = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);
	}

	void ImGuiLayer::setDarkThemeV2Colors()
	{
	}

	void ImGuiLayer::allowInputEvents(bool allowEvents)
	{
	}
}
