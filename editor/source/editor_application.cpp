#include "editor_application.hpp"
#include "editor_layer.hpp"
#include "imgui_layer.hpp"

namespace toaster
{
	EditorApplication::EditorApplication(const ApplicationCreateInfo &p_create_info, const CommandLineArgs *p_command_line_args) : Application(p_create_info,
																																			   p_command_line_args)
	{
		addLayer(IAppLayer::alloc<EditorLayer>(this));

		m_imGuiLayer = IAppLayer::alloc<ImGuiLayer>(this);
		addLayer(m_imGuiLayer);

		setBeginUIRenderCallback([&]() -> void { m_imGuiLayer->begin(); });
		setEndUIRenderCallback([&]() -> void { m_imGuiLayer->end(); });
	}

	EditorApplication::~EditorApplication() = default;

	auto EditorApplication::setBlockUIEvents(bool p_block_ui_events) const -> void
	{
		m_imGuiLayer->setBlockEvents(p_block_ui_events);
	}
}
