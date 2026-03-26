#pragma once

#include <string>
#include "gl_enums.hpp"

namespace gl
{
	inline std::string to_string(Error p_value)
	{
		switch (p_value)
		{
			case Error::eNoError: return "No Error";
			case Error::eInvalidEnum: return "Invalid Enum";
			case Error::eInvalidValue: return "Invalid Value";
			case Error::eInvalidOperation: return "Invalid Operation";
			case Error::eStackOverflow: return "Stack Overflow";
			case Error::eStackUnderflow: return "Stack Underflow";
			case Error::eOutOfMemory: return "Out Of Memory";
			case Error::eInvalidFramebufferOperation: return "Invalid Framebuffer Operation";
			case Error::eContextLost: return "Context Lost";
			case Error::eTableTooLarge: return "Table Too Large";
		}
		return "";
	}

	inline std::string to_string(DebugSource p_value)
	{
		switch (p_value)
		{
			case DebugSource::eAPI: return "API";
			case DebugSource::eWindowSystem: return "Window System";
			case DebugSource::eShaderCompiler: return "Shader Compiler";
			case DebugSource::eThirdParty: return "Third Party";
			case DebugSource::eApplication: return "Application";
			case DebugSource::eOther: return "Other";
		}
		return "";
	}

	inline std::string to_string(DebugType p_value)
	{
		switch (p_value)
		{
			case DebugType::eError: return "Error";
			case DebugType::eDeprecatedBehavior: return "Deprecated Behavior";
			case DebugType::eUndefinedBehavior: return "Undefined Behavior";
			case DebugType::ePortability: return "Portability";
			case DebugType::ePerformance: return "Performance";
			case DebugType::eMarker: return "Marker";
			case DebugType::ePushGroup: return "Push Group";
			case DebugType::ePopGroup: return "Pop Group";
			case DebugType::eOther: return "Other";
		}
		return "";
	}

	inline std::string to_string(DebugSeverity p_value)
	{
		switch (p_value)
		{
			case DebugSeverity::eHigh: return "High";
			case DebugSeverity::eMedium: return "Medium";
			case DebugSeverity::eLow: return "Low";
			case DebugSeverity::eNotification: return "Notification";
		}
		return "";
	}

	inline std::string to_string(DataType p_value)
	{
		switch (p_value)
		{
			case DataType::eHalfFloat: return "HalfFloat";
			case DataType::eFloat: return "Float";
			case DataType::eFloat_32_UnsignedInt_24_8_REV: return "Float_32_UnsignedInt_24_8_REV";
			case DataType::eDouble: return "Double";
			case DataType::eFixed: return "Fixed";
			case DataType::eByte: return "Byte";
			case DataType::eUnsignedByte: return "UnsignedByte";
			case DataType::eUnsignedByte_3_3_2: return "UnsignedByte_3_3_2";
			case DataType::eUnsignedByte_2_3_3_REV: return "UnsignedByte_2_3_3_REV";
			case DataType::eShort: return "Short";
			case DataType::eUnsignedShort: return "UnsignedShort";
			case DataType::eUnsignedShort_5_6_5: return "UnsignedShort_5_6_5";
			case DataType::eUnsignedShort_5_6_5_REV: return "UnsignedShort_5_6_5_REV";
			case DataType::eUnsignedShort_4_4_4_4: return "UnsignedShort_4_4_4_4";
			case DataType::eUnsignedShort_4_4_4_4_REV: return "UnsignedShort_4_4_4_4_REV";
			case DataType::eUnsignedShort_5_5_5_1: return "UnsignedShort_5_5_5_1";
			case DataType::eUnsignedShort_1_5_5_5_REV: return "UnsignedShort_1_5_5_5_REV";
			case DataType::eInt: return "Int";
			case DataType::eInt_2_10_10_10_Rev: return "Int_2_10_10_10_Rev";
			case DataType::eBool: return "Bool";
			case DataType::eUnsignedInt: return "UnsignedInt";
			case DataType::eUnsignedInt_8_8_8_8: return "UnsignedInt_8_8_8_8";
			case DataType::eUnsignedInt_8_8_8_8_REV: return "UnsignedInt_8_8_8_8_REV";
			case DataType::eUnsignedInt_10_10_10_2: return "UnsignedInt_10_10_10_2";
			case DataType::eUnsignedInt_2_10_10_10_REV: return "UnsignedInt_2_10_10_10_REV";
			case DataType::eUnsignedInt_24_8: return "UnsignedInt_24_8";
			case DataType::eUnsignedInt_10F_11F_11F_REV: return "UnsignedInt_10F_11F_11F_REV";
			case DataType::eUnsignedInt_5_9_9_9_REV: return "UnsignedInt_5_9_9_9_REV";
		}
		return "";
	}

	inline std::string to_string(Format p_value)
	{
		switch (p_value)
		{
			case Format::eRed: return "Red";
			case Format::eRG: return "RG";
			case Format::eRGB: return "RGB";
			case Format::eBGR: return "BGR";
			case Format::eRGBA: return "RGBA";
			case Format::eBGRA: return "BGRA";
			case Format::eRedInteger: return "RedInteger";
			case Format::eRGInteger: return "RGInteger";
			case Format::eRGBInteger: return "RGBInteger";
			case Format::eBGRInteger: return "BGRInteger";
			case Format::eRGBAInteger: return "RGBAInteger";
			case Format::eBGRAInteger: return "BGRAInteger";
			case Format::eStencilIndex: return "StencilIndex";
			case Format::eDepthComponent: return "DepthComponent";
			case Format::eDepthStencil: return "DepthStencil";
			case Format::eR8: return "R8";
			case Format::eR8Snorm: return "R8Snorm";
			case Format::eR16: return "R16";
			case Format::eR16Snorm: return "R16Snorm";
			case Format::eRG8: return "RG8";
			case Format::eRG8Snorm: return "RG8Snorm";
			case Format::eRG16: return "RG16";
			case Format::eRG16Snorm: return "RG16Snorm";
			case Format::eR3G3B2: return "R3G3B2";
			case Format::eRGB4: return "RGB4";
			case Format::eRGB5: return "RGB5";
			case Format::eRGB8: return "RGB8";
			case Format::eRGB8Snorm: return "RGB8Snorm";
			case Format::eRGB10: return "RGB10";
			case Format::eRGB12: return "RGB12";
			case Format::eRGB16Snorm: return "RGB16Snorm";
			case Format::eRGBA2: return "RGBA2";
			case Format::eRGBA4: return "RGBA4";
			case Format::eRGB5A1: return "RGB5A1";
			case Format::eRGBA8: return "RGBA8";
			case Format::eRGBA8Snorm: return "RGBA8Snorm";
			case Format::eRGB10A2: return "RGB10A2";
			case Format::eRGB10A2UI: return "RGB10A2UI";
			case Format::eRGBA12: return "RGBA12";
			case Format::eRGBA16: return "RGBA16";
			case Format::eSRGB8: return "SRGB8";
			case Format::eSRGB8Alpha8: return "SRGB8Alpha8";
			case Format::eR16F: return "R16F";
			case Format::eRG16F: return "RG16F";
			case Format::eRGB16F: return "RGB16F";
			case Format::eRGBA16F: return "RGBA16F";
			case Format::eR11FG11FB10F: return "R11FG11FB10F";
			case Format::eRGB9E5: return "RGB9E5";
			case Format::eR8I: return "R8I";
			case Format::eR8UI: return "R8UI";
			case Format::eR16I: return "R16I";
			case Format::eR16UI: return "R16UI";
			case Format::eR32I: return "R32I";
			case Format::eR32UI: return "R32UI";
			case Format::eRG8I: return "RG8I";
			case Format::eRG8UI: return "RG8UI";
			case Format::eRG16I: return "RG16I";
			case Format::eRG16UI: return "RG16UI";
			case Format::eRG32I: return "RG32I";
			case Format::eRG32UI: return "RG32UI";
			case Format::eRGB8I: return "RGB8I";
			case Format::eRGB8UI: return "RGB8UI";
			case Format::eRGB16I: return "RGB16I";
			case Format::eRGB16UI: return "RGB16UI";
			case Format::eRGB32I: return "RGB32I";
			case Format::eRGB32UI: return "RGB32UI";
			case Format::eRGBa8I: return "RGBa8I";
			case Format::eRGBa8UI: return "RGBa8UI";
			case Format::eRGBa16I: return "RGBa16I";
			case Format::eRGBa16UI: return "RGBa16UI";
			case Format::eRGBa32I: return "RGBa32I";
			case Format::eRGBa32UI: return "RGBa32UI";
			case Format::eCompressedRed: return "CompressedRed";
			case Format::eCompressedRG: return "CompressedRG";
			case Format::eCompressedRGB: return "CompressedRGB";
			case Format::eCompressedRGBA: return "CompressedRGBA";
			case Format::eCompressedSRGB: return "CompressedSRGB";
			case Format::eCompressedSRGBAlpha: return "CompressedSRGBAlpha";
			case Format::eCompressedRedRGTC1: return "CompressedRedRGTC1";
			case Format::eCompressedSignedRedRGTC1: return "CompressedSignedRedRGTC1";
			case Format::eCompressedRGRGTC2: return "CompressedRGRGTC2";
			case Format::eCompressedSignedRGRGTC2: return "CompressedSignedRGRGTC2";
			case Format::eCompressedRGBABPTCUnorm: return "CompressedRGBABPTCUnorm";
			case Format::eCompressedSRGBAlphaBPTCUnorm: return "CompressedSRGBAlphaBPTCUnorm";
			case Format::eCompressedRGBBPTCSignedFloat: return "CompressedRGBBPTCSignedFloat";
			case Format::eCompressedRGBBPTCUnsignedFloat: return "CompressedRGBBPTCUnsignedFloat";
			case Format::eDepthComponent16: return "DepthComponent16";
			case Format::eDepthComponent24: return "DepthComponent24";
			case Format::eDepthComponent32: return "DepthComponent32";
			case Format::eDepthComponent32F: return "DepthComponent32F";
			case Format::eDepth24Stencil8: return "Depth24Stencil8";
			case Format::eDepth32FStencil8: return "Depth32FStencil8";
		}
		return "";
	}
}
