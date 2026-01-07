#pragma once

#include <span>
#include <vector>
#include "system_types.h"

namespace toaster::gpu
{
	using ShaderBytecode = std::span<const uint32>;
}
