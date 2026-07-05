#pragma once

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
}
