#include "device_manager.hpp"

#include "memory/allocator.hpp"
#include "vulkan/vulkan_device_manager.hpp"

namespace tst::gpu
{
	DeviceManager *DeviceManager::create(nvrhi::GraphicsAPI graphics_api)
	{
		switch (graphics_api)
		{
				#ifdef TST_ENABLE_VULKAN
			case nvrhi::GraphicsAPI::VULKAN:
			{
				return tnew VulkanDeviceManager();
			}
				#endif

				#ifdef TST_ENABLE_D3D11
			case nvrhi::GraphicsAPI::D3D12:
			{
				return tnew
				D3D11DeviceManager();
			}
				#endif

				#ifdef TST_ENABLE_D3D12
			case nvrhi::GraphicsAPI::D3D12:
			{
				return tnew
				D3D12DeviceManager();
			}
				#endif

			default:
			{
				return nullptr;
			}
		}
	}


}
