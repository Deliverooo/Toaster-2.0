#include "editor_application.hpp"
#include "editor_layer.hpp"
#include "imgui_layer.hpp"

namespace toaster
{
	EditorApplication::EditorApplication(const ApplicationCreateInfo& p_create_info) : Application(p_create_info)
	{

		addLayer(IAppLayer::alloc<EditorLayer>(this));

		m_imGuiLayer = IAppLayer::alloc<ImGuiLayer>(this);
		addLayer(m_imGuiLayer);

		setBeginUIRenderCallback([&]() -> void { m_imGuiLayer->begin(); });
		setEndUIRenderCallback([&]() -> void { m_imGuiLayer->end(); });
	}

	EditorApplication::~EditorApplication() = default;

	void EditorApplication::setBlockUIEvents(bool p_block_ui_events)
	{
		m_imGuiLayer->setBlockEvents(p_block_ui_events);
	}
}
