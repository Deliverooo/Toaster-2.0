#pragma once
#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	class ImGuiLayer;

	class EditorApplication final : public Application
	{
	public:
		explicit EditorApplication(const ApplicationSpecInfo &p_create_info, const CommandLineArgs *p_command_line_args);
		~EditorApplication();

		auto setBlockUIEvents(bool p_block_ui_events) const -> void;

	private:
		ImGuiLayer *m_imGuiLayer{nullptr};
	};
}
