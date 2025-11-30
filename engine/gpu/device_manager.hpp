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

#include "core/error.hpp"
#include "core/core_typedefs.hpp"

#include <nvrhi/vulkan.h>
#include <nvrhi/nvrhi.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef TST_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

namespace tst::gpu
{
	struct DeviceSpecInfo
	{
		uint32 refreshRate            = 0u;
		uint32 swapChainBufferCount   = 3u;
		uint32 swapChainSampleCount   = 1u;
		uint32 swapChainSampleQuality = 0u;
		uint32 maxFramesInFlight      = 2u;

		nvrhi::Format swapchainFormat = nvrhi::Format::RGBA8_UNORM;

		bool vSyncEnabled = false;
	};

	class DeviceManager
	{
	public:
		static DeviceManager *create(nvrhi::GraphicsAPI graphics_api);

		DeviceManager()          = default;
		virtual ~DeviceManager() = default;

		virtual EError createDevice(const DeviceSpecInfo &info) = 0;
		virtual void   destroyDevice() = 0;

		[[nodiscard]] virtual nvrhi::IDevice *   getDevice() const = 0;
		[[nodiscard]] virtual nvrhi::GraphicsAPI getGraphicsAPI() const = 0;

		const DeviceSpecInfo &getSpecInfo() { return m_specInfo; }

	protected:
		DeviceSpecInfo     m_specInfo;
		nvrhi::GraphicsAPI m_graphicsAPI;
	};
}
