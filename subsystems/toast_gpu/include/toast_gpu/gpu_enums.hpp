#pragma once

#include "toast_lib/util_defines.hpp"

namespace toaster::gpu
{
	enum class EPrimitiveTopology
	{
		ePointList,
		eLineList,
		eLineStrip,
		eTriangleList,
		eTriangleStrip,
		eTriangleFan,
		eLineListWithAdjacency,
		eLineStripWithAdjacency,
		eTriangleListWithAdjacency,
		eTriangleStripWithAdjacency,
		ePatchList
	};

	enum class ECullMode
	{
		eNone,
		eFront,
		eBack,
		eFrontAndBack
	};

	enum class EFrontFace
	{
		eCCW, eCW
	};

	enum class EPolygonMode
	{
		eFill, eLine, ePoint
	};

	enum class ECompareOp
	{
		eNever,
		eLess,
		eEqual,
		eLessOrEqual,
		eGreater,
		eNotEqual,
		eGreaterOrEqual,
		eAlways
	};

	enum class EIndexType
	{
		eUint16 = 0, eUint32 = 1, eUint8 = 1000265000
	};

	enum class ESampleCount
	{
		e1  = BIT(0),
		e2  = BIT(1),
		e4  = BIT(2),
		e8  = BIT(3),
		e16 = BIT(4),
		e32 = BIT(6),
		e64 = BIT(7)
	};
}
