#pragma once
#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	class EditorApplication final : public Application
	{
	public:
		explicit EditorApplication(const ApplicationCreateInfo &p_create_info, int32 p_argc, char * *p_argv);
		~EditorApplication();

		auto setBlockUIEvents(bool p_block_ui_events) const -> void;

	private:
		class ImGuiLayer *m_imGuiLayer{nullptr};
	};
}
