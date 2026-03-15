#pragma once
#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	class EditorApplication : public Application
	{
	public:
		EditorApplication();
		~EditorApplication();

		void setBlockUIEvents(bool p_block_ui_events);

	private:
		class ImGuiLayer *m_imGuiLayer{nullptr};
	};
}
