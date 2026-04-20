#pragma once

#include "toast_lib/core_basic.hpp"

#include <unordered_set>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace toaster::gpu
{
	class VKInstance
	{
	public:
		#ifndef NDEBUG
		static constexpr bool c_enableValidationLayers{true};
		#else
		static constexpr bool c_enableValidationLayers{false};
		#endif

		VKInstance();

		// After configuration, call this to initialise the Vulkan library... :)
		auto create() -> void;

		[[nodiscard]] auto getVulkanInstance() -> vk::raii::Instance &;

		auto setRequiredExtensions(const std::unordered_set<CString> &p_extensions) -> void;
		auto setDebugCallback(vk::PFN_DebugUtilsMessengerCallbackEXT p_callback) -> void;

	private:
		vk::raii::Context m_context;

		std::unordered_set<CString> m_requiredExtensions;
		vk::raii::Instance          m_vulkanInstance{nullptr};

		vk::PFN_DebugUtilsMessengerCallbackEXT m_debugCallback{nullptr};
		vk::raii::DebugUtilsMessengerEXT       m_debugUtilsMessenger{nullptr};
	};
}
