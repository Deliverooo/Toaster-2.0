#include "editor_application.hpp"
#include "editor_layer.hpp"
#include "imgui_layer.hpp"

namespace toaster
{
	EditorApplication::EditorApplication() : Application()
	{
		io::filesystem::setWorkingDirectory("../"); // The main Toaster dir (where the resource folder is)

		__super::addLayer(IAppLayer::alloc<EditorLayer>(this));

		m_imGuiLayer = IAppLayer::alloc<ImGuiLayer>(this);
		__super::addLayer(m_imGuiLayer);

		__super::setBeginUIRenderCallback([&]() -> void { m_imGuiLayer->begin(); });
		__super::setEndUIRenderCallback([&]() -> void { m_imGuiLayer->end(); });
	}

	EditorApplication::~EditorApplication() = default;

	void EditorApplication::setBlockUIEvents(bool p_block_ui_events)
	{
		m_imGuiLayer->setBlockEvents(p_block_ui_events);
	}
}
