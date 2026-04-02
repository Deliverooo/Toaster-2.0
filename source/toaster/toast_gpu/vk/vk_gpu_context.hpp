#pragma once

#include "../gpu_context.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/string.hpp"
#include "toast_lib/system_types.h"

#include <unordered_set>

namespace toaster::gpu
{
	#ifdef NDEBUG
	constexpr bool c_enableValidationLayers{false};
	#else
	constexpr bool c_enableValidationLayers{true};
	#endif

	class VKGPUContext final : public IGPUContext
	{
	public:
		constexpr static uint32 c_maxFramesInFlight{3u};

		VKGPUContext(GLFWwindow *p_window);
		~VKGPUContext() noexcept override;

		[[nodiscard]] vk::raii::Instance &      getVulkanInstance();
		[[nodiscard]] vk::raii::PhysicalDevice &getPhysicalDevice();
		[[nodiscard]] vk::raii::Device &        getDevice();
		[[nodiscard]] vk::raii::Queue &         getGraphicsQueue();
		[[nodiscard]] vk::raii::SurfaceKHR &    getSurface();

		[[nodiscard]] vk::raii::CommandPool &  getCommandPool();
		[[nodiscard]] vk::raii::CommandBuffer &getCommandBuffer(uint32 p_index);

		[[nodiscard]] vk::raii::Pipeline &getGraphicsPipeline();

		void drawFrame();

		void transitionImageLayout(vk::Image &             p_image, uint32                     p_frame_index, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2        p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2 p_dst_stage_mask);

	private:
		void _createInstance();
		void _createDebugMessenger();
		void _createSurface();
		void _pickPhysicalDevice();
		void _createLogicalDevice();

		void _createGraphicsPipeline();
		void _createCommandPool();
		void _createCommandBuffer();

		void _recreateSwapchain();

		bool _isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const;

		std::vector<CString> _getRequiredInstanceExtensions() const;

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity,
															   vk::DebugUtilsMessageTypeFlagsEXT             p_message_type,
															   const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);

		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};

		GLFWwindow *         m_window{nullptr}; // Used as a reference to create the window surface
		vk::raii::SurfaceKHR m_surface{nullptr};

		vk::raii::PhysicalDevice m_currentPhysicalDevice{nullptr};

		struct QueueFamilyIndices
		{
			uint32 graphics{UINT32_MAX};
		};

		std::vector<CString> m_requiredDeviceExtensions{vk::KHRSwapchainExtensionName};
		vk::raii::Device     m_device{nullptr};

		vk::raii::Queue    m_graphicsQueue{nullptr};
		QueueFamilyIndices m_queueFamilyIndices{};

		vk::raii::ShaderModule _createShaderModule(const std::vector<uint8> &p_code);

		vk::raii::Pipeline       m_graphicsPipeline{nullptr};
		vk::raii::PipelineLayout m_pipelineLayout{nullptr};

		vk::raii::CommandPool                m_commandPool{nullptr};
		std::vector<vk::raii::CommandBuffer> m_commandBuffers;
	};
}
