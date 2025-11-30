#pragma once

#include "core/core.hpp"
#include "platform/window.hpp"

namespace tst
{
	class Main
	{
	public:
		Main(std::vector<String> args);
		~Main();

		EError init();
		bool   loop();

		Window &getPrimaryWindow() { return *m_primaryWindow; }

		static Main &getInstance() { return *s_instance; }

	private:
		std::unique_ptr<Window> m_primaryWindow;
		float32                 m_deltaTime;

		static inline Main *s_instance = nullptr;
	};
}
