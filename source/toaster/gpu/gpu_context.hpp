#pragma once

#include <unordered_set>

#ifndef VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#endif
#include <vulkan/vulkan.hpp>

#include "system_types.h"

#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>

struct GLFWwindow;

namespace toaster
{
	inline std::vector<const char *> stringSetToVector(const std::unordered_set<std::string> &set)
	{
		std::vector<const char *> ret;
		for (const auto &s: set)
		{
			ret.push_back(s.c_str());
		}

		return ret;
	}
}

namespace toaster::gpu
{
	class GPUContext
	{
	public:
		static constexpr uint32 c_maxFramesInFlight{3};

		// Only creates the Vulkan instance and debug messenger, so any operations (such as the window surface creation)
		// should be called after this and before init()
		GPUContext();
		~GPUContext();

		// needed because when picking a physical device, a window surface is required
		// to query present support and the window surface should not be created in the context itself...
		void init(vk::SurfaceKHR p_window_surface);

		[[nodiscard]] vk::Instance getInstance() const;

		[[nodiscard]] bool                                   isValidationEnabled() const;
		[[nodiscard]] bool                                   isValidationLayerSupported(const std::string &p_layer_name) const;
		[[nodiscard]] const std::unordered_set<std::string> &getEnabledValidationLayers() const;

		[[nodiscard]] bool                                   isInstanceExtensionEnabled(const std::string &p_extension_name) const;
		[[nodiscard]] bool                                   isInstanceExtensionSupported(const std::string &p_extension_name) const;
		[[nodiscard]] const std::unordered_set<std::string> &getEnabledInstanceExtensions() const;

		[[nodiscard]] bool                                   isDeviceExtensionEnabled(const std::string &p_extension_name) const;
		[[nodiscard]] bool                                   isDeviceExtensionSupported(const std::string &p_extension_name) const;
		[[nodiscard]] const std::unordered_set<std::string> &getEnabledDeviceExtensions() const;

		struct QueueFamilyIndices
		{
			int32 graphics = -1;
			int32 present  = -1;
			int32 transfer = -1;
			int32 compute  = -1;

			// todo: maybe check for transfer and compute support
			[[nodiscard]] bool isComplete() const { return (graphics != -1) && (present != -1); }
		};

		[[nodiscard]] vk::PhysicalDevice           getPhysicalDevice() const;
		[[nodiscard]] vk::PhysicalDeviceFeatures   getPhysicalDeviceFeatures() const;
		[[nodiscard]] vk::PhysicalDeviceProperties getPhysicalDeviceProperties() const;
		[[nodiscard]] const QueueFamilyIndices &   getQueueFamilyIndices() const;
		[[nodiscard]] uint32                       findMemoryTypeIndex(uint32 p_type_filter, vk::MemoryPropertyFlags p_flags) const;

		[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format> &p_candidates, vk::ImageTiling p_tiling, vk::FormatFeatureFlags p_features) const;
		[[nodiscard]] vk::Format findDepthFormat() const;
		[[nodiscard]] bool       hasStencilComponent(vk::Format p_format) const;

		[[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice p_physical_device,
														   vk::SurfaceKHR     p_window_surface /* Needed to query present support*/) const;
		[[nodiscard]] bool checkDeviceExtensionSupport(vk::PhysicalDevice p_physical_device) const;

		struct SwapchainSupportDetails
		{
			vk::SurfaceCapabilitiesKHR        capabilities;
			std::vector<vk::SurfaceFormatKHR> formats;
			std::vector<vk::PresentModeKHR>   presentModes;
		};

		[[nodiscard]] SwapchainSupportDetails querySwapchainSupport(vk::PhysicalDevice p_physical_device, vk::SurfaceKHR p_window_surface) const;
		[[nodiscard]] vk::SurfaceFormatKHR    chooseSwapchainFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const;
		[[nodiscard]] vk::PresentModeKHR      choosePresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const;
		vk::Extent2D                          chooseExtent(const vk::SurfaceCapabilitiesKHR &p_capabilities, GLFWwindow *p_target_window) const;

		[[nodiscard]] vk::Device getLogicalDevice() const;
		[[nodiscard]] vk::Queue  getGraphicsQueue() const;
		[[nodiscard]] vk::Queue  getPresentQueue() const;

		[[nodiscard]] nvrhi::IDevice *getNVRHIDevice() const;
		[[nodiscard]] nvrhi::Format   getSwapchainFormat() const;

	private:
		void _createInstance();
		void _setupDebug();
		void _pickPhysicalDevice(vk::SurfaceKHR p_window_surface);
		void _createLogicalDevice();
		void _createNVRHIObjects();

		GLFWwindow *m_pWindow{nullptr};

		vk::Instance m_vulkanInstance{nullptr};

		[[nodiscard]] bool              _checkValidationLayerSupport() const;
		std::unordered_set<std::string> m_enabledValidationLayers{"VK_LAYER_KHRONOS_validation"};
		#ifdef NDEBUG
		bool m_enableValidationLayers = false;
		#else
		bool m_enableValidationLayers = true;
		#endif

		std::unordered_set<std::string> m_enabledInstanceExtensions{
			#ifndef NDEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			#endif
		};
		std::unordered_set<std::string> m_optionalInstanceExtensions{};

		static VkBool32 _debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
									   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *                            pUserData);
		VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};

		vk::PhysicalDevice m_physicalDevice;
		QueueFamilyIndices m_queueFamilyIndices;

		[[nodiscard]] bool _isDeviceSuitable(vk::PhysicalDevice p_physical_device, vk::SurfaceKHR p_window_surface /* Needed to query present support*/) const;

		vk::Device                      m_logicalDevice;
		std::unordered_set<std::string> m_enabledDeviceExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
			VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME,
			VK_KHR_MAINTENANCE1_EXTENSION_NAME,
		};
		std::unordered_set<std::string> m_optionalDeviceExtensions{VK_NV_FILL_RECTANGLE_EXTENSION_NAME};

		vk::Queue m_graphicsQueue{nullptr};
		vk::Queue m_presentQueue{nullptr};

		nvrhi::vulkan::DeviceHandle m_nvrhiDevice;

		nvrhi::Format m_swapchainFormat{nvrhi::Format::SBGRA8_UNORM};
	};
}
