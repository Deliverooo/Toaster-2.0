#pragma once

#include "toast_render.hpp"
#include "toast_gpu/device.hpp"

namespace toaster::render
{
	// The render context must be destroyed before the device to ensure proper resource cleanup
	class TST_RENDER_API RenderContext
	{
		TST_REGISTER_DEPENDENCY(gpu::Device, Device, device)
	public:
		RenderContext(gpu::Device & p_device);
		~RenderContext();

	private:
	};
}
