#pragma once

#define GLFW_INCLUDE_VULKAN
#include <deque>
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "vk_instance.hpp"
#include "vk_logical_device.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	struct VKGPUContextSpecInfo
	{
	};

	class VKGPUContext final
	{
	public:
		constexpr static uint32 c_maxFramesInFlight{3u};

		struct QueueFamilyIndices
		{
			uint32 graphics{UINT32_MAX};
			uint32 transfer{UINT32_MAX};
			uint32 compute{UINT32_MAX};
		};

		VKGPUContext(VKLogicalDevice *p_logical_device, const VKGPUContextSpecInfo &p_spec_info);

		// If you care about not getting crashes when allocating gpu objects mid-frame, you should use this instead of make_reference<Type>(ctx, ...)
		template<typename Type, typename... TArgs>
		auto alloc(TArgs &&... p_args) -> RefPtr<Type>
		{
			return allocate_reference<Type>([this](Type *p_ptr) -> void
			{
				auto deleter{
					[p_ptr]() -> void
					{
						delete p_ptr;
					}
				};
				m_pendingDeletions[m_currentFrameIndex].emplace_back(std::move(deleter));
			}, this, std::forward<TArgs>(p_args)...);
		}

		template<typename TLambda>
		auto submitResourceUpdate(TLambda &&p_func) -> void
		{
			m_pendingResourceUpdates[m_currentFrameIndex].emplace_back(p_func);
		}

		// Only the swapchain should use this, but I don't want to make it private and friend it because "Coupling"...
		auto setCurrentFrameIndex(uint32 p_index) -> void;

		auto performGarbageCollection() -> void;

		auto getSpecInfo() const -> const VKGPUContextSpecInfo &;

		[[nodiscard]] auto getInstance() const -> VKInstance *;
		[[nodiscard]] auto getPhysicalDevice() -> VKPhysicalDevice *;
		[[nodiscard]] auto getLogicalDevice() -> VKLogicalDevice *;

		auto createShaderModule(const std::vector<uint8> &p_code) -> vk::raii::ShaderModule;
		auto createShaderModule(const std::vector<uint32> &p_code) -> vk::raii::ShaderModule;

		[[nodiscard]] auto findMemoryType(uint32 p_type_filter, vk::MemoryPropertyFlags p_properties) const -> uint32;

		auto createBuffer(vk::DeviceSize          p_size, vk::BufferUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::raii::Buffer &p_out_buffer,
						  vk::raii::DeviceMemory &p_out_memory) const -> void;

		auto copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) const -> void;

		auto createImage(uint32 p_width, uint32 p_height, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count, vk::Format p_format,
						 vk::ImageTiling p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::raii::Image &p_out_image,
						 vk::raii::DeviceMemory &p_out_memory) const -> void;

		auto transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::Image &          p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2    p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2  p_dst_stage_mask, vk::ImageAspectFlags p_aspect_flags) -> void;

		auto transitionImageLayout(vk::raii::CommandBuffer &p_command_buffer, vk::raii::Image &    p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2    p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2  p_dst_stage_mask, vk::ImageAspectFlags p_aspect_flags) -> void;

		auto transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
								   vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
								   uint32           p_mip_levels, vk::ImageAspectFlags p_aspect_flags) const -> void;

		auto copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, uint32 p_width, uint32 p_height) const -> void;

		[[nodiscard]] auto createImageView(vk::raii::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags,
										   uint32           p_mip_levels) const -> vk::raii::ImageView;
		[[nodiscard]] auto createImageView(vk::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags,
										   uint32     p_mip_levels) const -> vk::raii::ImageView;

		auto generateMipmaps(vk::raii::Image &p_src_image, vk::Format p_format, uint32 p_width, uint32 p_height, uint32 p_mip_levels) const -> void;

		[[nodiscard]] auto beginSingleTimeCommandsTransfer() const -> vk::raii::CommandBuffer;
		auto               endSingleTimeCommandsTransfer(vk::raii::CommandBuffer &p_command_buffer) const -> void;

		[[nodiscard]] auto beginSingleTimeCommandsGraphics() const -> vk::raii::CommandBuffer;
		auto               endSingleTimeCommandsGraphics(vk::raii::CommandBuffer &p_command_buffer) const -> void;

		[[nodiscard]] auto hasStencilComponent(vk::Format p_format) const -> bool;
		[[nodiscard]] auto isDepthFormat(vk::Format p_format) const -> bool;

	private:
		VKLogicalDevice *m_logicalDevice{nullptr};

		VKGPUContextSpecInfo m_specInfo{};

		std::array<std::deque<std::function<void()> >, c_maxFramesInFlight> m_pendingDeletions;
		std::array<std::deque<std::function<void()> >, c_maxFramesInFlight> m_pendingResourceUpdates;
		uint32                                                              m_currentFrameIndex{0};
	};
}
