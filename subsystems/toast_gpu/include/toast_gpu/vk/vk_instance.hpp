#pragma once

#include "../toast_gpu.hpp"

#include "toast_lib/core_basic.hpp"

#include <unordered_set>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	using ExtensionSet = std::unordered_set<String>;

	struct TST_GPU_API VKInstanceSpecInfo
	{
		ExtensionSet requiredExtensions; // These are INSTANCE extensions
		String       appName{};

		bool enableValidationLayers{true};
	};

	class TST_GPU_API VKInstance
	{
	public:
		VKInstance(const VKInstanceSpecInfo &p_spec_info);

		[[nodiscard]] auto getVulkanInstance() -> vk::raii::Instance &;

		operator vk::raii::Instance &() { return m_vulkanInstance; }

	private:
		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};
	};
}
