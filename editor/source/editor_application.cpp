#include "editor_application.hpp"
#include "editor_layer.hpp"
#include "imgui_layer.hpp"

namespace toaster
{
	EditorApplication::EditorApplication() : Application()
	{
		__super::addLayer(IAppLayer::alloc<EditorLayer>(this));

		const auto imgui_layer = IAppLayer::alloc<ImGuiLayer>(this);
		__super::addLayer(imgui_layer);

		__super::setBeginUIRenderCallback([&]() -> void { imgui_layer->begin(); });
		__super::setEndUIRenderCallback([&]() -> void { imgui_layer->end(); });
	}

	EditorApplication::~EditorApplication() = default;
}
