#pragma once

#include "toaster/toast_kernel/layer.hpp"

#include <imgui.h>

namespace toaster
{
	class ImGuiLayer : public IAppLayer
	{
	public:
		ImGuiLayer(Application *p_app);
		~ImGuiLayer() override = default;

		void onInit() override;
		void onDestroy() override;

		void onUpdate(float32 p_dt) override;
		void onEvent(Event &p_event) override;

		void begin();
		void end();

	private:
	};
}
