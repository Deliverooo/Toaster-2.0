#include "imgui_layer.hpp"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	ImGuiLayer::ImGuiLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ImGuiLayer::onInit()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

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
	}
}
