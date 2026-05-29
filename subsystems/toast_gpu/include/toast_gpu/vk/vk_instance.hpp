#pragma once

#include "../toast_gpu.hpp"

#include "toast_lib/core_basic.hpp"

#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	struct TST_GPU_API VKInstanceSpecInfo
	{
		using ExtensionSet = std::unordered_set<String>;

		String                                 appName{};
		ExtensionSet                           requiredExtensions;
		vk::PFN_DebugUtilsMessengerCallbackEXT debugCallback{nullptr};

		bool enableValidationLayers{true};
		bool printDebugInfo{true};
	};

	class TST_GPU_API VKInstance
	{
	public:
		VKInstance(const VKInstanceSpecInfo &p_spec_info);

		[[nodiscard]] auto getSpecInfo() const -> const VKInstanceSpecInfo &;
		[[nodiscard]] auto getVulkanInstance() -> vk::raii::Instance &;

		operator vk::raii::Instance &() { return m_vulkanInstance; }

	private:
		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		VKInstanceSpecInfo m_specInfo;

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};
	};
}
