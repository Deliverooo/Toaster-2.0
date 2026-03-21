#include "gpu_api.hpp"

#include "gl/gl_gpu_api.hpp"

namespace toaster::gpu
{
	std::unique_ptr<IGPUAPI> IGPUAPI::create()
	{
		return std::make_unique<GLGPUAPI>();
	}
}
