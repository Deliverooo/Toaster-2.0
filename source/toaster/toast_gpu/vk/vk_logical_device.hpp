#pragma once

#include "vk_common.hpp"
#include "vk_physical_device.hpp"
#include <deque>

#include "toast_lib/command_queue.hpp"

namespace toaster::gpu
{
	struct VKLogicalDeviceSpecInfo
	{
		static auto getDefaultFeatures() -> vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT>
		{
			vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceCustomBorderColorFeaturesEXT> feature_chain{{}, {}, {}, {}, {}};
			feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy                 = true;
			feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading                 = true;
			feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid                  = true;
			feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fragmentStoresAndAtomics          = true;
			feature_chain.get<vk::PhysicalDeviceFeatures2>().features.vertexPipelineStoresAndAtomics    = true;
			feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                   = true;
			feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                    = true;
			feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2                    = true;
			feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
			feature_chain.get<vk::PhysicalDeviceCustomBorderColorFeaturesEXT>().customBorderColors      = true;
			return feature_chain;
		}

		using ExtensionSet = std::unordered_set<String>;

		ExtensionSet requiredExtensions;

		// Optional :)
		bool usePresent{false};

		// Goto vk_shader.cpp #define TST_SHADER_LOG_TRACE for usage... if true, prints shader reflection data
		bool printShaderDebugInfo{true};

		uint32 maxFramesInFlight{3u};

		// Use ts to set your logical device features using vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features...
		void *pNext{nullptr};
	};

	class TST_GPU_API VKLogicalDevice
	{
	public:
		struct QueueFamilyIndices
		{
			uint32 graphics{UINT32_MAX};
			uint32 transfer{UINT32_MAX};
			uint32 compute{UINT32_MAX};
		};

		VKLogicalDevice(VKPhysicalDevice *p_physical_device, const VKLogicalDeviceSpecInfo &p_spec_info);

		template<typename TFunc>
		auto deferDestruction(TFunc &&p_func) -> void
		{
			m_pendingDeletionCommandQueues[m_currentFrameIndex].enqueue(std::forward<TFunc>(p_func));
		}

		auto setCurrentFrameIndex(uint32 p_index) -> void;
		auto performGarbageCollection() -> void;

		[[nodiscard]] auto getPhysicalDevice() const -> NonOwningPtr<VKPhysicalDevice>;
		[[nodiscard]] auto getSpecInfo() const -> const VKLogicalDeviceSpecInfo &;

		[[nodiscard]] auto getVulkanLogicalDevice() -> vk::raii::Device &;

		[[nodiscard]] auto getQueueFamilyIndices() const -> const QueueFamilyIndices &;
		[[nodiscard]] auto getGraphicsQueue() -> vk::raii::Queue &;
		[[nodiscard]] auto getTransferQueue() -> vk::raii::Queue &;
		[[nodiscard]] auto getComputeQueue() -> vk::raii::Queue &;
		[[nodiscard]] auto getQueue(vk::QueueFlagBits p_queue_type) -> vk::raii::Queue &;

		[[nodiscard]] auto getGraphicsCommandPool() -> vk::raii::CommandPool &;
		[[nodiscard]] auto getTransferCommandPool() -> vk::raii::CommandPool &;
		[[nodiscard]] auto getComputeCommandPool() -> vk::raii::CommandPool &;
		[[nodiscard]] auto getCommandPool(vk::QueueFlagBits p_queue_type) -> vk::raii::CommandPool &;

		auto waitForFence(const vk::Fence &p_fence, uint64 p_timeout = UINT64_MAX) const -> void;
		auto waitForFences(const std::initializer_list<const vk::Fence> &p_fences, bool p_wait_all = true, uint64 p_timeout = UINT64_MAX) const -> void;

		[[nodiscard]] auto createShaderModule(const std::vector<uint8> &p_code) -> vk::raii::ShaderModule;
		[[nodiscard]] auto createShaderModule(const std::vector<uint32> &p_code) -> vk::raii::ShaderModule;

		#pragma region vulkan non-raii wrappers
		auto mapMemory(vk::DeviceMemory p_memory, vk::DeviceSize p_offset, vk::DeviceSize p_size, vk::MemoryMapFlags p_flags) const -> void *;
		auto unmapMemory(vk::DeviceMemory p_memory) const -> void;

		template<typename TVKObj>
		auto destroyObject(TVKObj &p_obj) const -> void
		{
			TST_PERMA_ASSERT(false);
		}
		#pragma endregion

		auto createBuffer(vk::DeviceSize          p_size, vk::BufferUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::raii::Buffer &p_out_buffer,
						  vk::raii::DeviceMemory &p_out_memory) -> void;
		auto createBuffer(vk::DeviceSize    p_size, vk::BufferUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::Buffer &p_out_buffer,
						  vk::DeviceMemory &p_out_memory) -> void;

		auto createImage(const ImageExtent &p_image_extent, uint32 p_layer_count, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count, vk::Format p_format,
						 vk::ImageTiling p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::raii::Image &p_out_image,
						 vk::raii::DeviceMemory &p_out_memory) -> void;

		auto createImage(const ImageExtent &p_image_extent, uint32 p_layer_count, uint32 p_mip_levels, vk::SampleCountFlagBits p_sample_count, vk::Format p_format,
						 vk::ImageTiling    p_image_tiling, vk::ImageUsageFlags p_usage_flags, vk::MemoryPropertyFlags p_memory_properties, vk::Image &p_out_image,
						 vk::DeviceMemory & p_out_memory) -> void;
		[[nodiscard]] auto createImageView(vk::raii::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_layer_count,
										   uint32           p_mip_levels) -> vk::raii::ImageView;
		[[nodiscard]] auto createImageView(vk::Image &p_src_image, vk::Format p_format, vk::ImageAspectFlags p_aspect_flags, uint32 p_layer_count,
										   uint32     p_mip_levels) -> vk::ImageView;
		[[nodiscard]] auto createSamplerRaii() -> vk::raii::Sampler;
		[[nodiscard]] auto createSampler() -> vk::Sampler;

		auto copyBuffer(vk::raii::Buffer &p_src_buffer, vk::raii::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void;
		auto copyBuffer(const vk::Buffer &p_src_buffer, const vk::Buffer &p_dst_buffer, vk::DeviceSize p_size) -> void;

		auto copyBufferToImage(vk::raii::Buffer &p_src_buffer, vk::raii::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void;
		auto copyBufferToImage(const vk::Buffer &p_src_buffer, const vk::Image &p_dst_image, const ImageExtent &p_image_extent, uint32 p_layer_count) -> void;

		auto transitionImageLayout(vk::raii::Image &p_image, const ImageLayoutInfo &   p_src_layout_info, const ImageLayoutInfo &p_dst_layout_info, uint32 p_layer_count,
								   uint32           p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;
		auto transitionImageLayout(vk::Image &p_image, const ImageLayoutInfo &   p_src_layout_info, const ImageLayoutInfo &p_dst_layout_info, uint32 p_layer_count,
								   uint32     p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;

		auto transitionImageLayout(vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
								   vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
								   uint32 p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;
		auto transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::raii::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2 p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2 p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;
		auto transitionImageLayout(vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout, vk::AccessFlags2 p_src_access_mask,
								   vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask, vk::PipelineStageFlags2 p_dst_stage_mask,
								   uint32 p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;
		auto transitionImageLayout(vk::raii::CommandBuffer &p_cmd, vk::Image &p_image, vk::ImageLayout p_old_layout, vk::ImageLayout p_new_layout,
								   vk::AccessFlags2         p_src_access_mask, vk::AccessFlags2 p_dst_access_mask, vk::PipelineStageFlags2 p_src_stage_mask,
								   vk::PipelineStageFlags2  p_dst_stage_mask, uint32 p_layer_count, uint32 p_mip_levels, vk::ImageAspectFlags p_aspect_flags) -> void;

		auto generateMipmaps(vk::raii::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void;
		auto generateMipmaps(vk::Image &p_src_image, const ImageExtent &p_image_extent, uint32 p_mip_levels) -> void;

		operator vk::raii::Device &() { return m_logicalDevice; }

	private:
		NonOwningPtr<VKPhysicalDevice> m_physicalDevice{nullptr};

		VKLogicalDeviceSpecInfo m_specInfo{};

		vk::raii::Device m_logicalDevice{nullptr};

		QueueFamilyIndices m_queueFamilyIndices{};
		vk::raii::Queue    m_graphicsQueue{nullptr};
		vk::raii::Queue    m_transferQueue{nullptr};
		vk::raii::Queue    m_computeQueue{nullptr};

		vk::raii::CommandPool m_graphicsCommandPool{nullptr};
		vk::raii::CommandPool m_transferCommandPool{nullptr};
		vk::raii::CommandPool m_computeCommandPool{nullptr};

		std::vector<CommandQueue> m_pendingDeletionCommandQueues;
		uint32                    m_currentFrameIndex{0u};
	};

	#pragma region destroy

	template<>
	inline auto VKLogicalDevice::destroyObject<vk::DeviceMemory>(vk::DeviceMemory &p_memory) const -> void
	{
		((vk::Device) m_logicalDevice).freeMemory(p_memory);
	}

	template<>
	inline auto VKLogicalDevice::destroyObject<vk::Buffer>(vk::Buffer &p_buffer) const -> void
	{
		((vk::Device) m_logicalDevice).destroyBuffer(p_buffer);
	}

	template<>
	inline auto VKLogicalDevice::destroyObject<vk::Image>(vk::Image &p_image) const -> void
	{
		((vk::Device) m_logicalDevice).destroyImage(p_image);
	}

	template<>
	inline auto VKLogicalDevice::destroyObject<vk::ImageView>(vk::ImageView &p_image_view) const -> void
	{
		((vk::Device) m_logicalDevice).destroyImageView(p_image_view);
	}

	template<>
	inline auto VKLogicalDevice::destroyObject<vk::Sampler>(vk::Sampler &p_sampler) const -> void
	{

		((vk::Device) m_logicalDevice).destroySampler(p_sampler);
	}
	#pragma endregion
}
