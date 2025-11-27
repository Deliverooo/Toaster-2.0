#pragma once

#include "string/string.hpp"

namespace tst::gpu
{
	enum class EDeviceVendor
	{
		eUnknown = 0,
		eAMD,
		eApple,
		eIntel,
		eNvidia, };

	enum class EDeviceType
	{
		eUnknown = 0,
		eIntegratedGPU,
		eDiscreteGPU,
		eCPU, };

	struct Device
	{
		String        name   = "Null";
		EDeviceVendor vendor = EDeviceVendor::eUnknown;
		EDeviceType   device = EDeviceType::eUnknown;
	};
}
