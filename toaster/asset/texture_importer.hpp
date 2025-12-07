#pragma once

#include "buffer.hpp"
#include "gpu/image.hpp"

#include "filesystem/filesystem.hpp"

namespace tst
{
	class TextureImporter
	{
	public:
		static Buffer toBufferFromFile(const std::filesystem::path &path, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight);
		static Buffer toBufferFromMemory(Buffer buffer, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight);

	private:
		const std::filesystem::path m_Path;
	};
}
