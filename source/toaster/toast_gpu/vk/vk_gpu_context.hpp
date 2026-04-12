#pragma once

#include "../gpu_context.hpp"

#define GLFW_INCLUDE_VULKAN
#include <deque>
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	#ifndef NDEBUG
	constexpr bool c_enableValidationLayers{true};
	#else
	constexpr bool c_enableValidationLayers{false};
	#endif

	class VKGPUContext final : public IGPUContext
	{
	public:
		constexpr static uint32 c_maxFramesInFlight{3u};

		struct QueueFamilyIndices
		{
			uint32 graphics{UINT32_MAX};
			uint32 transfer{UINT32_MAX};
			uint32 compute{UINT32_MAX};
		};

		VKGPUContext(GLFWwindow *p_window);
		~VKGPUContext() noexcept override;

		// If you care about not getting crashes when allocating gpu objects mid-frame, you should use this instead of make_reference<Type>(ctx, ...)
		template<typename Type, typename... TArgs>
		RefPtr<Type> alloc(TArgs &&... p_args)
		{
			return allocate_reference<Type>([this](Type *p_ptr) -> void
			{
				auto deleter = [p_ptr]() -> void
				{
					delete p_ptr;
				};
				m_pendingDeletions[m_currentFrameIndex].emplace_back(std::move(deleter));
			}, this, std::forward<TArgs>(p_args)...);
		}

		// Only the swapchain should use this, but I don't want to make it private and friend it because "Coupling"...
		void setCurrentFrameIndex(uint32 p_index);

		void performGarbageCollection();

		[[nodiscard]] vk::raii::Instance &      getVulkanInstance();
		[[nodiscard]] vk::raii::PhysicalDevice &getPhysicalDevice();
		[[nodiscard]] vk::raii::Device &        getDevice();
		[[nodiscard]] vk::raii::Queue &         getGraphicsQueue();
		[[nodiscard]] vk::raii::Queue &         getTransferQueue();
		[[nodiscard]] vk::raii::Queue &         getComputeQueue();
		[[nodiscard]] const QueueFamilyIndices &getQueueFamilyIndices() const;
		[[nodiscard]] vk::raii::SurfaceKHR &    getSurface();

		[[nodiscard]] vk::raii::CommandPool &getGraphicsCommandPool();
		[[nodiscard]] vk::raii::CommandPool &getTransferCommandPool();
		[[nodiscard]] vk::raii::CommandPool &getComputeCommandPool();

		vk::raii::ShaderModule createShaderModule(const std::vector<uint8> &p_code);
		vk::raii::ShaderModule createShaderModule(const std::vector<uint32> &p_code);

		[[nodiscard]] uint32 findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const;

		void createBuffer(vk::DeviceSize          p_size, vk::BufferUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::raii::Buffer &p_out_buffer,
						  vk::raii::DeviceMemory &p_out_memory) const;

		void copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) const;

		void createImage(uint32 p_width, uint32 p_height, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count, vk::Format p_format,
						 vk::ImageTiling p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::raii::Image &p_out_image,
						 vk::raii::DeviceMemory &p_out_memory) const;

		void transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::Image &          p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2    p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2  p_dst_stage_mask, vk::ImageAspectFlags p_aspect_flags);

		void transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::raii::Image &    p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2    p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2  p_dst_stage_mask, vk::ImageAspectFlags p_aspect_flags);

		void transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags p_src_access_mask,
								   vk::AccessFlags  p_dst_access_mask, vk::PipelineStageFlags p_src_stage_mask, vk::PipelineStageFlags p_dst_stage_mask,
								   uint32           p_mip_levels, vk::ImageAspectFlags p_aspect_flags) const;

		void copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, uint32 p_width, uint32 p_height) const;

		[[nodiscard]] vk::raii::ImageView createImageView(vk::raii::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags,
														  uint32           p_mip_levels) const;
		[[nodiscard]] vk::raii::ImageView createImageView(vk::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_mip_levels) const;

		void generateMipmaps(vk::raii::Image &p_src_image, vk::Format p_format, uint32 p_width, uint32 p_height, uint32 p_mip_levels) const;

		[[nodiscard]] vk::raii::CommandBuffer beginSingleTimeCommandsTransfer() const;
		void                                  endSingleTimeCommandsTransfer(vk::raii::CommandBuffer &p_command_buffer) const;

		[[nodiscard]] vk::raii::CommandBuffer beginSingleTimeCommandsGraphics() const;
		void                                  endSingleTimeCommandsGraphics(vk::raii::CommandBuffer &p_command_buffer) const;

		[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format> &p_supported_formats, vk::ImageTiling p_tiling,
													 vk::FormatFeatureFlags         p_feature_flags) const;
		[[nodiscard]] vk::Format findDepthFormat() const;
		[[nodiscard]] bool       hasStencilComponent(vk::Format p_format) const;
		[[nodiscard]] bool       isDepthFormat(vk::Format p_format) const;

		[[nodiscard]] vk::SampleCountFlagBits getMaxUsableSampleCount() const;

	private:
		void _createInstance();
		void _createDebugMessenger();
		void _createSurface();
		void _pickPhysicalDevice();
		void _createLogicalDevice();
		void _createCommandPools();

		[[nodiscard]] bool _isDeviceSuitable(const vk::raii::PhysicalDevice &p_physical_device) const;

		[[nodiscard]] std::vector<CString> _getRequiredInstanceExtensions() const;

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL _debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      p_message_severity,
															   vk::DebugUtilsMessageTypeFlagsEXT             p_message_type,
															   const vk::DebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);

		std::array<std::deque<std::function<void()> >, c_maxFramesInFlight> m_pendingDeletions;
		uint32                                                              m_currentFrameIndex{0};

		vk::raii::Context  m_context;
		vk::raii::Instance m_vulkanInstance{nullptr};

		vk::raii::DebugUtilsMessengerEXT m_debugUtilsMessenger{nullptr};

		GLFWwindow *         m_window{nullptr}; // Used as a reference to create the window surface
		vk::raii::SurfaceKHR m_surface{nullptr};

		vk::raii::PhysicalDevice m_currentPhysicalDevice{nullptr};

		const std::array<CString, 4> m_requiredDeviceExtensions{
			vk::KHRSwapchainExtensionName,
			vk::KHRDynamicRenderingExtensionName,
			vk::KHRTimelineSemaphoreExtensionName,
			vk::EXTCustomBorderColorExtensionName
		};
		vk::raii::Device m_device{nullptr};

		vk::raii::Queue    m_graphicsQueue{nullptr};
		vk::raii::Queue    m_transferQueue{nullptr};
		vk::raii::Queue    m_computeQueue{nullptr};
		QueueFamilyIndices m_queueFamilyIndices{};

		vk::raii::CommandPool m_graphicsCommandPool{nullptr};
		vk::raii::CommandPool m_transferCommandPool{nullptr};
		vk::raii::CommandPool m_computeCommandPool{nullptr};

		vk::Format              m_depthFormat{vk::Format::eUndefined};
		vk::SampleCountFlagBits m_maxUsableSampleCount{vk::SampleCountFlagBits::e1};
	};
}
