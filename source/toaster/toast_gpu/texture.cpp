#include "gl/gl_texture.hpp"

namespace toaster::gpu
{
	RefPtr<ITexture2D> ITexture2D::create(uint32 p_width, uint32 p_height)
	{
		return make_reference<GLTexture2D>(p_width, p_height);
	}

	RefPtr<ITexture2D> ITexture2D::create(const io::filesystem::Path &p_path)
	{
		return make_reference<GLTexture2D>(p_path);
	}
}
