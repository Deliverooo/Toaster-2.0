#include "client_application.hpp"
#include "client_layer.hpp"
#include "imgui_layer.hpp"

namespace toaster
{
	ClientApplication::ClientApplication(const ApplicationCreateInfo& p_create_info) : Application(p_create_info)
	{
		addLayer(IAppLayer::alloc<ClientLayer>(this));

		m_imGuiLayer = IAppLayer::alloc<ImGuiLayer>(this);
		addLayer(m_imGuiLayer);

		setBeginUIRenderCallback([&]() -> void { m_imGuiLayer->begin(); });
		setEndUIRenderCallback([&]() -> void { m_imGuiLayer->end(); });
	}

	ClientApplication::~ClientApplication()
	{
	}
}
