#include "framebuffer.hpp"
#include "gl/gl_framebuffer.hpp"

namespace toaster::gpu
{
	RefPtr<IFramebuffer> IFramebuffer::create(const FramebufferCreateInfo &p_framebuffer_create_info)
	{
		return make_reference<GLFramebuffer>(p_framebuffer_create_info);
	}
}
