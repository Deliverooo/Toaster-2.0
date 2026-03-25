#include "imgui_layer.hpp"

#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "toaster/toast_kernel/application.hpp"

#include <GLFW/glfw3.h>

#include "ui/colours.hpp"

namespace toaster
{
	ImGuiLayer::ImGuiLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ImGuiLayer::onInit()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO &io = ImGui::GetIO();

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.FontDefault = io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\UDDigiKyokashoN-R.ttc)", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		io.Fonts->AddFontFromFileTTF(R"(C:\Windows\Fonts\UDDigiKyokashoN-B.ttc)", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		// io.FontDefault = io.Fonts->AddFontFromFileTTF("resources/fonts/DejaVuSans/DejaVuSans.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		// io.Fonts->AddFontFromFileTTF("resources/fonts/DejaVuSans/DejaVuSans-Bold.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

		ImGui_ImplGlfw_InitForOpenGL(getApp().getWindow().getNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460 core");

		auto &style  = ImGui::GetStyle();
		auto &colours = ImGui::GetStyle().Colors;

		// Headers
		colours[ImGuiCol_Header]        = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);
		colours[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);
		colours[ImGuiCol_HeaderActive]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);

		// Buttons
		colours[ImGuiCol_Button]        = ImColor(56, 56, 56, 200);
		colours[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
		colours[ImGuiCol_ButtonActive]  = ImColor(56, 56, 56, 150);

		// Frame BG
		colours[ImGuiCol_FrameBg]        = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::propertyField);
		colours[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::propertyField);
		colours[ImGuiCol_FrameBgActive]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::propertyField);

		// Tabs
		colours[ImGuiCol_Tab]                = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TabHovered]         = ImColor(255, 225, 135, 30);
		colours[ImGuiCol_TabActive]          = ImColor(255, 225, 135, 60);
		colours[ImGuiCol_TabUnfocused]       = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TabUnfocusedActive] = colours[ImGuiCol_TabHovered];

		// Title
		colours[ImGuiCol_TitleBg]          = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TitleBgActive]    = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

		// Resize Grip
		colours[ImGuiCol_ResizeGrip]        = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colours[ImGuiCol_ResizeGripActive]  = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colours[ImGuiCol_ScrollbarBg]          = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colours[ImGuiCol_ScrollbarGrab]        = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colours[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colours[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

		// Slider
		colours[ImGuiCol_SliderGrab]       = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colours[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colours[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::text);

		// Checkbox
		colours[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::text);

		// Separator
		colours[ImGuiCol_Separator]        = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundDark);
		colours[ImGuiCol_SeparatorActive]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::highlight);
		colours[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

		// Window Background
		colours[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::titlebar);
		colours[ImGuiCol_ChildBg]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::background);
		colours[ImGuiCol_PopupBg]  = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundPopup);
		colours[ImGuiCol_Border]   = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundDark);

		// Tables
		colours[ImGuiCol_TableHeaderBg]    = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::groupHeader);
		colours[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(ui::colours::theme::backgroundDark);

		// Menubar
		colours[ImGuiCol_MenuBarBg] = ImVec4{0.0f, 0.0f, 0.0f, 0.0f};

		/// Style
		style.FrameRounding   = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing   = 11.0f;
	}

	void ImGuiLayer::onDestroy()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::onUpdate(float32 p_dt)
	{
	}

	void ImGuiLayer::onEvent(Event &p_event)
	{
		if (m_blockEvents)
		{
			ImGuiIO &io = ImGui::GetIO();
			p_event.setHandled(p_event.isHandled() | (p_event.inCategory(EventCategory_Mouse) & io.WantCaptureMouse));
			p_event.setHandled(p_event.isHandled() | (p_event.inCategory(EventCategory_Keyboard) & io.WantCaptureKeyboard));
		}
	}

	void ImGuiLayer::begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::end()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		ImGuiIO &io = ImGui::GetIO();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow *context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(context);
		}
	}

	void ImGuiLayer::setBlockEvents(bool p_block)
	{
		m_blockEvents = p_block;
	}
}
