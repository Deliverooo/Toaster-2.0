#pragma once

#include "logical_device.hpp"

namespace toaster::gpu
{
	class TST_GPU_API Swapchain
	{
	public:
		Swapchain(LogicalDevice &p_logical_device, vk::SurfaceKHR p_surface);

		auto getVulkanSwapchain() const -> const vk::raii::SwapchainKHR & { return m_swapchain; }
		auto operator *() const -> const vk::raii::SwapchainKHR & { return m_swapchain; }

		auto getCurrentImage() -> vk::Image & { return m_images[m_imageIndex]; }
		auto getCurrentImageView() -> vk::ImageView & { return m_imageViews[m_imageIndex]; }
		auto getCurrentCommandBuffer() -> vk::raii::CommandBuffer & { return m_commandBuffers[m_frameIndex]; }

		auto getExtent() const -> vk::Extent2D { return m_extent; }
		auto getFrameIndex() const -> uint32 { return m_frameIndex; }
		auto getImageIndex() const -> uint32 { return m_imageIndex; }

		auto beginFrame() -> void;
		auto endFrame() -> void;
		auto presentFrame() -> void;

	private:
		auto _createSwapchain() -> void;
		auto _createSwapchainObjects() -> void;
		auto _recreateSwapchain() -> void;

		vk::raii::SwapchainKHR m_swapchain{nullptr};
		vk::SurfaceKHR         m_windowSurface{nullptr};

		std::vector<vk::Image>     m_images;
		std::vector<vk::ImageView> m_imageViews;

		std::vector<vk::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::Semaphore> m_renderFinishedSemaphores;
		std::vector<vk::Fence>     m_inFlightFences;

		std::vector<vk::raii::CommandBuffer> m_commandBuffers;

		vk::Extent2D m_extent{};

		uint32 m_frameIndex{0u};
		uint32 m_imageIndex{0u};
	};
}
