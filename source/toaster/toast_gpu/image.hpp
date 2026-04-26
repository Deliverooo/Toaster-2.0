#pragma once

namespace toaster::gpu
{
	enum class EImageFormat
	{
		eR8UNorm,
		eR8UInt,
		eR16UInt,
		eR32UInt,
		eR32F,
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
		eDepth32FStencil8UInt,
		eDepth32F,
		eDepth24Stencil8
	};

	enum class EImageUsage
	{
		eTexture,
		eAttachment,
		eStorage,
		eHostRead
	};

	enum class ETextureFiltering
	{
		eLinear, eNearest, eCubic
	};

	enum class ETextureWrapping
	{
		eClamp, eRepeat
	};
}
