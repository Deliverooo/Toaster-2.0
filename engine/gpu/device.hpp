/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
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
		eNvidia
	};

	enum class EDeviceType
	{
		eUnknown = 0,
		eIntegratedGPU,
		eDiscreteGPU,
		eCPU
	};

	struct Device
	{
		String        name   = "Null";
		EDeviceVendor vendor = EDeviceVendor::eUnknown;
		EDeviceType   device = EDeviceType::eUnknown;
	};
}
