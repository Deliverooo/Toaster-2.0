#include "imgui_layer.hpp"

#include <filesystem>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "toaster/toast_kernel/application.hpp"

#include <GLFW/glfw3.h>

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

		// io.Fonts->AddFontFromFileTTF("resources/fonts/Roboto/Roboto-Regular.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\UDDigiKyokashoN-R.ttc", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		// io.Fonts->AddFontFromFileTTF("resources/fonts/DejaVuSans/DejaVuSans.ttf", 16.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());

		ImGui_ImplGlfw_InitForOpenGL(getApp().getWindow().getNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 460 core");
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
