#pragma once
#include "toaster/toast_kernel/application.hpp"

namespace toaster
{
	class ClientApplication : public Application
	{
	public:
		ClientApplication(const ApplicationCreateInfo& p_create_info);
		~ClientApplication();

	private:
		class ImGuiLayer *m_imGuiLayer{nullptr};
	};
}
