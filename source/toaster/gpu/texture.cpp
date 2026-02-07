#include "gl/gl_texture.hpp"

namespace toaster::gpu
{
	RefPtr<Texture2D> Texture2D::create(const io::filesystem::Path &p_path)
	{
		return std::make_shared<GLTexture2D>(p_path);
	}
}
