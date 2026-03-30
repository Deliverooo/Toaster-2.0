#pragma once

namespace toaster::gpu
{
	enum class EImageFormat
	{
		eInvalid = 0,
		eRed8UN,
		eRed8UI,
		eRed16UI,
		eRed32UI,
		eRed32F,
		eRG8,
		eRG16F,
		eRG32F,
		eRGB,
		eRGBA,
		eRGBA16F,
		eRGBA32F,
		eB10R11G11UF,
		eSRGB,
		eSRGBA,

		// Depth Formats
		eDepth32FStencil8UInt,
		eDepth32F,
		eDepth24Stencil8
	};

	inline bool isDepthFormat(const EImageFormat p_format)
	{
		return (p_format == EImageFormat::eDepth32FStencil8UInt) || (p_format == EImageFormat::eDepth32F) || (p_format == EImageFormat::eDepth24Stencil8);
	}
}
