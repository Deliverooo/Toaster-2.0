#pragma once
#include "toaster/toast_kernel/application.hpp"

#include <qapplication.h>

namespace toaster
{
	class ClientApplication : public Application
	{
	public:
		ClientApplication(const ApplicationCreateInfo &p_create_info, int32 p_argc, char **p_argv);
		~ClientApplication();

	private:
		class ImGuiLayer *m_imGuiLayer{nullptr};

		QGuiApplication *m_qApplication{nullptr};
	};
}
