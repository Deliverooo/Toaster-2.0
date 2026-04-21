#pragma once

#include "toast_lib/core_basic.hpp"

#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	struct VKInstanceSpecInfo
	{
		using ExtensionSet = std::unordered_set<CString>;

		String                                 appName{};
		ExtensionSet                           requiredExtensions;
		vk::PFN_DebugUtilsMessengerCallbackEXT debugCallback{nullptr};

		bool enableValidationLayers{true};
	};

	class VKInstance
	{
	public:
		VKInstance(const VKInstanceSpecInfo &p_spec_info);

		auto               getSpecInfo() const -> const VKInstanceSpecInfo &;
		[[nodiscard]] auto getVulkanInstance() -> vk::raii::Instance &;

	private:
		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		VKInstanceSpecInfo m_specInfo;

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};
	};
}
