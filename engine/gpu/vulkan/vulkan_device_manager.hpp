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

#include "gpu/device_manager.hpp"
#include "string/string.hpp"
#include "logging/log.hpp"

#include <unordered_set>

#include <nvrhi/vulkan.h>
#include <nvrhi/validation.h>

#include <vulkan/vulkan.hpp>

namespace tst::gpu
{
	struct QueueFamilyIndices
	{
		int32 graphics = -1;
		int32 compute  = -1;
		int32 transfer = -1;
		int32 present  = -1;
	};

	struct DefaultMessageCallback : public nvrhi::IMessageCallback
	{
		static DefaultMessageCallback &get();

		void message(nvrhi::MessageSeverity severity, const char *messageText) override;
	};

	class VulkanDeviceManager : public DeviceManager
	{
	public:
		VulkanDeviceManager();
		~VulkanDeviceManager() override;

		[[nodiscard]] nvrhi::IDevice *getDevice() const override
		{
			return m_nvrhiDevice;
		}

		[[nodiscard]] nvrhi::GraphicsAPI getGraphicsAPI() const override
		{
			return nvrhi::GraphicsAPI::VULKAN;
		}

		EError createDevice(const DeviceSpecInfo &info) override;
		void   destroyDevice() override;

		[[nodiscard]] vk::Instance       getVulkanInstance() const { return m_vulkanInstance; }
		[[nodiscard]] vk::PhysicalDevice getVulkanPhysicalDevice() const { return m_vulkanPhysicalDevice; }

		bool isVulkanInstanceExtensionEnabled(const char *extensionName) const
		{
			return m_enabledInstanceExtensions.contains(extensionName);
		}

		bool isVulkanDeviceExtensionEnabled(const char *extensionName) const
		{
			return m_enabledDeviceExtensions.contains(extensionName);
		}

		bool isVulkanValidationLayerEnabled(const char *layerName) const
		{
			return m_enabledValidationLayers.contains(layerName);
		}

		void getEnabledVulkanInstanceExtensions(std::vector<std::string> &extensions) const
		{
			for (const auto &ext: m_enabledInstanceExtensions)
				extensions.push_back(ext);
		}

		void getEnabledVulkanDeviceExtensions(std::vector<std::string> &extensions) const
		{
			for (const auto &ext: m_enabledDeviceExtensions)
				extensions.push_back(ext);
		}

		void getEnabledVulkanValidationLayers(std::vector<std::string> &layers) const
		{
			for (const auto &ext: m_enabledValidationLayers)
				layers.push_back(ext);
		}

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL _vulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t        obj, size_t location,
																   int32_t               code, const char *                layerPrefix, const char *msg, void * userData)
		{
			TST_WARN_TAG("Renderer", "[Vulkan: location=0x{0:x} code={1}, layerPrefix='{2}'] {3}\n", location, code, layerPrefix, msg);

			return VK_FALSE;
		}

		EError _createDevice();
		EError _createVulkanInstance();
		EError _createVulkanDevice();
		EError _pickPhysicalDevice();
		bool   _findQueueFamilies(vk::PhysicalDevice physical_device);

		std::unordered_set<String> m_enabledInstanceExtensions = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
		std::unordered_set<String> m_enabledValidationLayers   = {};
		std::unordered_set<String> m_enabledDeviceExtensions   = {VK_KHR_MAINTENANCE1_EXTENSION_NAME};

		std::unordered_set<String> m_optionalInstanceExtensions = {VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
		std::unordered_set<String> m_optionalValidationLayers   = {};
		std::unordered_set<String> m_optionalDeviceExtensions   = {
			VK_EXT_DEBUG_MARKER_EXTENSION_NAME,VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,VK_NV_MESH_SHADER_EXTENSION_NAME,
			VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME,VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,VK_KHR_MAINTENANCE_4_EXTENSION_NAME,
			VK_KHR_SWAPCHAIN_MUTABLE_FORMAT_EXTENSION_NAME
		};

		QueueFamilyIndices m_queueFamilyIndices;

		vk::Instance               m_vulkanInstance;
		vk::DebugReportCallbackEXT m_debugReportCallback;

		vk::PhysicalDevice m_vulkanPhysicalDevice;

		vk::Device m_vulkanDevice;
		vk::Queue  m_graphicsQueue;
		vk::Queue  m_computeQueue;
		vk::Queue  m_transferQueue;
		vk::Queue  m_presentQueue;

		vk::detail::DynamicLoader m_dynamicLoader;

		nvrhi::vulkan::DeviceHandle m_nvrhiDevice;

		bool m_bufferDeviceAddressSupported    = false;
		bool m_swapchainMutableFormatSupported = false;

		friend class VulkanSwapChain;
	};
}
