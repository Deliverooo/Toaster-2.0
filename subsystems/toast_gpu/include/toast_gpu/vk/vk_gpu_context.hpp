#pragma once

#include "vk_instance.hpp"
#include "vk_logical_device.hpp"

namespace toaster::gpu
{
	class VKDescriptorHeap;

	struct TST_GPU_API GPUContextSpecInfo
	{
		std::unordered_set<String> instanceExtensions; // Get with Window::getRequiredInstanceExtensions()

		bool32 printDebugInfo{false};

		uint32 maxFramesInFlight{3u}; // I don't know if this should be determined at runtime, so I am keeping it here just in case
	};

	class TST_GPU_API VKGPUContext
	{
	public:
		VKGPUContext(const GPUContextSpecInfo &p_spec_info);
		~VKGPUContext();

		[[nodiscard]] auto getSpecInfo() const -> const GPUContextSpecInfo &;

		[[nodiscard]] auto getBackendInstance() const -> VKInstance *;
		[[nodiscard]] auto getPhysicalDevice() const -> VKPhysicalDevice *;
		[[nodiscard]] auto getLogicalDevice() const -> VKLogicalDevice *;

		[[nodiscard]] auto getDescriptorHeap() const -> VKDescriptorHeap *; // Look at vk_ext_descriptor_heap docs for a general overview

		// Keeps track of the current frame index for use in -PFF gpu objects
		[[nodiscard]] auto getCurrentFrameIndex() const -> uint32;

		// Typically, if using a swapchain, this will be the swapchain's current command buffer
		[[nodiscard]] auto getCurrentCommandBuffer() const -> VKCommandBuffer *;

		// Go to vk_swapchain.hpp for usage of how and when these should be called
		auto setCurrentFrameIndex(uint32 p_index) -> void;
		auto setCurrentCommandBuffer(VKCommandBuffer *p_cmd) -> void;

		// All GPU objects defer their destruction to the deletion queue, once it safe to do so. E.g. after the previous frame's fence is acquired,
		// call this function to properly delete the objects and avoid issues.
		auto performGarbageCollection() -> void; // This is also called in the destructor of this class

		// Realistically, the client should really not be calling this, but it may have some use being made public...
		template<typename TFunc>
		auto deferDestruction(TFunc &&p_func) -> void
		{
			m_pendingDeletionCommandQueues[m_currentFrameIndex].enqueue(std::forward<TFunc>(p_func));
		}

		// From now on, if you see VKCommandBuffer* p_command_buffer = nullptr. It means that if no command buffer is provided,
		// it defaults to m_currentCommandBuffer... :)
		auto bindDescriptorHeap(VKCommandBuffer *p_command_buffer = nullptr) const -> void; // For compute shaders, you may need a separate cmd buffer

		auto copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void;
		auto copyBuffer(const vk::Buffer &p_src_buffer, const vk::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void;

		auto copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void;
		auto copyBufferToImage(const vk::Buffer &p_src_buffer, const vk::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void;

		auto copyImageToBuffer(vk::raii::Image &p_src_image, vk::raii::Buffer &p_dst_buffer, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void;
		auto copyImageToBuffer(const vk::Image &p_src_image, const vk::Buffer &p_dst_buffer, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void;

		auto transitionImageLayout(vk::raii::Image &p_image, const ImageLayoutInfo &   p_src_layout_info, const ImageLayoutInfo &p_dst_layout_info, uint32 p_layer_count,
								   uint32           p_mip_levels, vk::ImageAspectFlags p_aspect_flags, vk::CommandBuffer p_override_command_buffer = nullptr) -> void;
		auto transitionImageLayout(vk::Image &p_image, const ImageLayoutInfo &   p_src_layout_info, const ImageLayoutInfo &p_dst_layout_info, uint32 p_layer_count,
								   uint32     p_mip_levels, vk::ImageAspectFlags p_aspect_flags, vk::CommandBuffer         p_override_command_buffer = nullptr) -> void;

		auto transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
								   vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
								   uint32 p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;
		auto transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2 p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2 p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels,
								   vk::ImageAspectFlags p_aspect_flags) const -> void;
		auto transitionImageLayout(vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
								   vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
								   uint32 p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;
		auto transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::Image &                  p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2  p_dst_stage_mask, uint32            p_layer_count, uint32 p_mip_levels,
								   vk::ImageAspectFlags     p_aspect_flags) const -> void;

		auto generateMipmaps(vk::raii::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void;
		auto generateMipmaps(vk::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void;

	private:
		GPUContextSpecInfo m_specInfo{};

		OwningPtr<VKInstance>       m_backendInstance{nullptr};
		OwningPtr<VKPhysicalDevice> m_physicalDevice{nullptr};
		OwningPtr<VKLogicalDevice>  m_logicalDevice{nullptr};

		OwningPtr<VKDescriptorHeap> m_descriptorHeap{nullptr};

		std::vector<CommandQueue> m_pendingDeletionCommandQueues;
		uint32                    m_currentFrameIndex{0u};

		NonOwningPtr<VKCommandBuffer> m_currentCommandBuffer{nullptr};
	};
}
