#pragma once

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif

#include "../toast_gpu.hpp"

#include "toast_lib/core_basic.hpp"

#include <unordered_set>
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

	auto getDispatchLoader() -> vk::detail::DispatchLoaderDynamic;

	class TST_GPU_API VKInstance
	{
	public:
		VKInstance(const VKInstanceSpecInfo &p_spec_info);

		[[nodiscard]] auto getSpecInfo() const -> const VKInstanceSpecInfo &;
		[[nodiscard]] auto getVulkanInstance() -> vk::raii::Instance &;

		auto initDispatcher(vk::Device p_device) const -> void;

		operator vk::raii::Instance &() { return m_vulkanInstance; }

	private:
		VKInstanceSpecInfo m_specInfo;

		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};
	};
}
