#pragma once
#include "misc/layer.hpp"

namespace tst
{
	class EditorLayer : public ILayer
	{
	public:
		EditorLayer();
		~EditorLayer() override;

		void onInit() override;

		void onDestroy() override;

		void onUpdate(float32 dt) override;

		void onEvent(Event &event) override;

		void onGUIRender() override;

	private:
	};
}
