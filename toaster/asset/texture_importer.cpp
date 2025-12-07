#include "texture_importer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace tst
{
	Buffer TextureImporter::toBufferFromFile(const std::filesystem::path &path, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight)
	{
		EFileStatus fileStatus = FileSystem::tryOpenFileAndWait(path, 100);
		Buffer      imageBuffer;
		std::string pathString = path.string();
		bool        isSRGB     = (outFormat == ImageFormat::SRGB) || (outFormat == ImageFormat::SRGBA);

		int    width, height, channels;
		void * tmp;
		size_t size = 0;

		if (stbi_is_hdr(pathString.c_str()))
		{
			tmp = stbi_loadf(pathString.c_str(), &width, &height, &channels, 4);
			if (tmp)
			{
				size      = width * height * 4 * sizeof(float);
				outFormat = ImageFormat::RGBA32F;
			}
		}
		else
		{
			//stbi_set_flip_vertically_on_load(1);
			tmp = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
			if (tmp)
			{
				size      = width * height * 4;
				outFormat = isSRGB ? ImageFormat::SRGBA : ImageFormat::RGBA;
			}
		}

		if (!tmp)
		{
			return {};
		}

		TST_ASSERT(size > 0);
		imageBuffer.data = new byte[size]; // avoid `malloc+delete[]` mismatch.
		imageBuffer.size = size;
		std::memcpy(imageBuffer.data, tmp, size);
		stbi_image_free(tmp);

		outWidth  = width;
		outHeight = height;
		return imageBuffer;
	}

	Buffer TextureImporter::toBufferFromMemory(Buffer buffer, ImageFormat &outFormat, uint32_t &outWidth, uint32_t &outHeight)
	{
		Buffer imageBuffer;

		bool isSRGB = (outFormat == ImageFormat::SRGB) || (outFormat == ImageFormat::SRGBA);

		int    width, height, channels;
		void * tmp;
		size_t size;

		if (stbi_is_hdr_from_memory((const stbi_uc *) buffer.data, (int) buffer.size))
		{
			tmp       = (byte *) stbi_loadf_from_memory((const stbi_uc *) buffer.data, (int) buffer.size, &width, &height, &channels, STBI_rgb_alpha);
			size      = width * height * 4 * sizeof(float);
			outFormat = ImageFormat::RGBA32F;
		}
		else
		{
			// stbi_set_flip_vertically_on_load(1);
			tmp       = stbi_load_from_memory((const stbi_uc *) buffer.data, (int) buffer.size, &width, &height, &channels, STBI_rgb_alpha);
			size      = width * height * 4;
			outFormat = isSRGB ? ImageFormat::SRGBA : ImageFormat::RGBA;
		}

		imageBuffer.data = new byte[size]; // avoid `malloc+delete[]` mismatch.
		imageBuffer.size = size;
		std::memcpy(imageBuffer.data, tmp, size);
		stbi_image_free(tmp);

		if (!imageBuffer.data)
			return {};

		outWidth  = width;
		outHeight = height;
		return imageBuffer;
	}
}
