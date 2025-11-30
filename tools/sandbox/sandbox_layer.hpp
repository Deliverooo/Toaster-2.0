#pragma once
#include "misc/layer.hpp"

namespace tst
{
	class SandboxLayer : public ILayer
	{
	public:
		SandboxLayer();
		~SandboxLayer() override;

		void onInit() override;

		void onDestroy() override;

		void onUpdate(float32 dt) override;

		void onEvent(Event &event) override;

		void onGUIRender() override;

	private:
	};
}
