#include "texture.hpp"
#include "gl/gl_texture.hpp"

namespace toaster::gpu
{
	RefPtr<Texture> Texture::create(const io::filesystem::Path &p_path)
	{
		return std::make_shared<GLTexture>(p_path);
	}
}
