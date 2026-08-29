#pragma once

#include "toast_lib/system_types.h"
#include "toast_lib/util_defines.hpp"
#include "toast_lib/enum_flags.hpp"

namespace toaster::gpu
{
	TST_DECLARE_FLAGS_FOR_NAMESPACE()

	enum class EShaderStageBits : uint8
	{
		eVertex                 = TST_BIT(0u),
		ePixel                  = TST_BIT(1u),
		eCompute                = TST_BIT(2u),
		eGeometry               = TST_BIT(3u),
		eTessellationControl    = TST_BIT(4u),
		eTessellationEvaluation = TST_BIT(5u),
		eMesh                   = TST_BIT(6u),
		eTask                   = TST_BIT(7u)
	};

	TST_SPECIALISE_FLAGS(EShaderStageBits, EShaderStage);

	enum class EQueueType : uint8
	{
		eGraphics, eCompute, eTransfer
	};

	enum class ECommandPoolBits : uint8
	{
		eReset = TST_BIT(0u), // You should be resetting the whole pool instead!
		eTransient = TST_BIT(1u)
	};

	TST_SPECIALISE_FLAGS(ECommandPoolBits, ECommandPool);
}