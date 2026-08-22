#pragma once

#include "allocator.hpp"
#include "gpu_common.hpp"

namespace toaster::gpu
{
	class TST_GPU_API Swapchain
	{
		TST_REGISTER_DEPENDENCY(PhysicalDevice,PhysicalDevice, physicalDevice)
		TST_REGISTER_DEPENDENCY(LogicalDevice,Device, device)
		TST_REGISTER_DEPENDENCY(Allocator, Allocator, allocator)
	public:
		Swapchain(PhysicalDevice &p_physical_device, LogicalDevice &p_device, Allocator &p_allocator, vk::SurfaceKHR p_surface, uint32 p_max_frames_in_flight,
				  uint32          p_initial_width, uint32           p_initial_height);
		~Swapchain();

		auto beginFrame() -> void;
		auto endFrame() -> void;

		auto onResize(uint32 p_width, uint32 p_height) -> void;

		auto getExtent() const -> vk::Extent2D { return m_swapchainExtent; }
		auto getMinImageCount() const -> uint32 { return m_minImageCount; }
		auto getMaxFramesInFlight() const -> uint32 { return m_maxFramesInFlight; }
		auto getSurfaceFormat() const -> vk::SurfaceFormatKHR { return m_swapchainSurfaceFormat; }
		auto getDepthFormat() const -> vk::Format { return m_depthFormat; }

		auto getImageIndex() const -> uint32 { return m_imageIndex; }
		auto getFrameIndex() const -> uint32 { return m_frameIndex; }

		auto getCurrentImage() const -> vk::Image { return m_images[m_imageIndex]; }
		auto getCurrentImageView() const -> vk::ImageView { return m_imageViews[m_imageIndex]; }

		auto getDepthImage() const -> vk::Image { return m_depthImage; }
		auto getDepthImageView() const -> vk::ImageView { return m_depthImageView; }

		auto getCurrentCommandBuffer() const -> vk::CommandBuffer { return m_commandBuffers[m_frameIndex]; }

	private:
		auto _create(uint32 p_width, uint32 p_height) -> void;

		vk::Extent2D         m_swapchainExtent;
		uint32               m_minImageCount{0u};
		uint32               m_maxFramesInFlight{0u};
		vk::SurfaceFormatKHR m_swapchainSurfaceFormat;
		vk::Format           m_depthFormat{vk::Format::eUndefined};

		vk::SwapchainKHR m_swapchain{nullptr};
		vk::SurfaceKHR   m_windowSurface{nullptr};

		// PFF = Per frame in flight. PSI = Per swapchain image
		std::vector<vk::Image>     m_images;     // PSI
		std::vector<vk::ImageView> m_imageViews; // PSI

		vk::Image     m_depthImage{nullptr};
		VmaAllocation m_depthImageAllocation{nullptr};
		vk::ImageView m_depthImageView{nullptr};

		std::vector<vk::Semaphore> m_imageAvailableSemaphores; // PFF
		std::vector<vk::Semaphore> m_renderFinishedSemaphores; // PSI
		std::vector<vk::Fence>     m_inFlightFences;           // PFF

		std::vector<vk::CommandBuffer> m_commandBuffers; // PFF

		uint32 m_imageIndex{0u};
		uint32 m_frameIndex{0u};
	};
}
