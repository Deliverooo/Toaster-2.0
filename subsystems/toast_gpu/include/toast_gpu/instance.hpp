#pragma once

#include "toast_gpu.hpp"

// I hate ts
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_STORAGE_SHARED
#ifdef TST_GPU_BUILD_DLL
#ifdef TST_GPU_DLL_EXPORT
#define VULKAN_HPP_STORAGE_SHARED_EXPORT
#endif
#endif

#include <unordered_set>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

// I hate the Windows API :(
#undef min
#undef max

namespace toaster::gpu
{
	using ExtensionSet = std::unordered_set<CString>;

	struct TST_GPU_API InstanceDesc
	{
		ExtensionSet requiredExtensions; // These are INSTANCE extensions

		CString applicationName{"I don't know"};

		bool enableValidationLayers{true};
	};

	// Ok, because vulkan is weird, you will have to call these functions that initialise Vulkan's extension function pointers
	TST_GPU_API auto initBaseFunctions() -> void;                            // Call before creating the instance
	TST_GPU_API auto initInstanceFunctions(vk::Instance p_instance) -> void; // Call right after creating the instance
	TST_GPU_API auto initDeviceFunctions(vk::Device p_device) -> void;       // Call right after creating the logical device

	class TST_GPU_API Instance
	{
	public:
		Instance(const InstanceDesc &p_desc);
		~Instance();

		auto getInstance() const -> vk::Instance { return m_vulkanInstance; }

	private:
		vk::Instance m_vulkanInstance{nullptr};

		vk::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};
	};
}
