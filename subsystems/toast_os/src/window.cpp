#include "toast_os/window.hpp"

namespace toaster::os
{
	auto getWindowBackBufferSize(Wnd p_window, uint &p_out_width, uint &p_out_height) -> void
	{
		RECT window_rect{};
		GetClientRect(p_window, &window_rect);
		p_out_width  = static_cast<uint>(window_rect.right) - static_cast<uint>(window_rect.left);
		p_out_height = static_cast<uint>(window_rect.bottom) - static_cast<uint>(window_rect.top);
	}
}
