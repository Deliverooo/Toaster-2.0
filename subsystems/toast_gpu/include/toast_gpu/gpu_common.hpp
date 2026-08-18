#pragma once

#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	enum class EMemoryProperties : uint8
	{
		eDeviceLocal, eHostVisibleCoherent
	};
}
