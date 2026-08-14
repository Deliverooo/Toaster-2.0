#pragma once

#include "toast_gpu.hpp"

#include <unordered_set>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include <Windows.h>
#undef min
#undef max

namespace toaster::gpu
{
	using ExtensionSet = std::unordered_set<CString>;

	struct TST_GPU_API InstanceSpecInfo
	{
		ExtensionSet requiredExtensions; // These are INSTANCE extensions

		bool enableValidationLayers{true};
	};

	class TST_GPU_API Instance
	{
	public:
		Instance(const InstanceSpecInfo &p_spec_info);
		~Instance();

		auto getVulkanInstance() const -> const vk::Instance & { return m_vulkanInstance; }
		auto operator *() const -> const vk::Instance & { return m_vulkanInstance; }

	private:
		// vk::raii::Context  m_context;
		vk::Instance m_vulkanInstance{nullptr};

		vk::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};
	};
}
