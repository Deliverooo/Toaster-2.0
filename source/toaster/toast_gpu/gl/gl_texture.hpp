#pragma once

#include <openglhpp/opengl.hpp>

#include "toaster/toast_gpu/texture.hpp"

namespace toaster::gpu
{
	class GLTexture2D : public Texture2D
	{
	public:
		GLTexture2D(uint32 p_width, uint32 p_height);
		GLTexture2D(const io::filesystem::Path &p_path);
		~GLTexture2D() override;

		void setData(void *p_data, uint32 p_size) override;

		void bind(uint32 p_slot) const override;

		[[nodiscard]] uint32 getID() const override;

		[[nodiscard]] uint32 getWidth() const override;
		[[nodiscard]] uint32 getHeight() const override;

		bool operator==(const Texture &p_other) const override;

	private:
		io::filesystem::Path m_path;

		uint32 m_width{0u};
		uint32 m_height{0u};

		gl::UInt m_textureId{0u};

		gl::Format m_dataFormat;
		gl::Format m_internalFormat;
	};
}
