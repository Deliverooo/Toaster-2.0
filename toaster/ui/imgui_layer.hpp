#pragma once

#include "core/layer.hpp"
#include "core/allocator.hpp"

#include "imgui_renderer.hpp"

namespace tst
{
	class ImGuiLayer : public ILayer
	{
	public:
		ImGuiLayer()          = default;
		virtual ~ImGuiLayer() = default;

		static ImGuiLayer *create()
		{
			return tnew ImGuiLayer();
		}

		virtual void onInit() override;
		virtual void onDestroy() override;

		void begin();
		void end();

		void setDarkThemeColors();
		void setDarkThemeV2Colors();

		void allowInputEvents(bool allowEvents);

	private:
		void initPlatformInterface();

		std::unique_ptr<ImGuiRenderer> m_ImGuiRenderer;
	};
}
