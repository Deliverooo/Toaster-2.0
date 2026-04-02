#pragma once

#include "vk_gpu_context.hpp"

namespace toaster::gpu
{
	class VKSwapchain
	{
	public:
		VKSwapchain(VKGPUContext *p_ctx, GLFWwindow *p_window);
		~VKSwapchain();

		void beginFrame();
		void endFrame();

		uint32 getFrameIndex() const;
		uint32 getImageIndex() const;

		vk::Image &          getImage(uint32 p_index);
		vk::raii::ImageView &getImageView(uint32 p_index);

		vk::Extent2D getExtent() const;

		void resize(uint32 p_width, uint32 p_height);
	private:
		void _createSwapchain();
		void _createImageViews();
		void _createSyncObjects();

		uint32 _acquireNextImage();

		VKGPUContext *m_ctx{nullptr};
		GLFWwindow *  m_window{nullptr};

		vk::SurfaceFormatKHR _chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const;
		vk::PresentModeKHR   _chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const;
		vk::Extent2D         _chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const;
		uint32               _chooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const;

		vk::raii::SwapchainKHR           m_swapchain{nullptr};
		std::vector<vk::Image>           m_swapchainImages;
		std::vector<vk::raii::ImageView> m_swapchainImageViews;
		vk::SurfaceFormatKHR             m_swapchainSurfaceFormat;
		vk::Extent2D                     m_swapchainExtent;

		std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
		std::vector<vk::raii::Fence>     m_inFlightFences;

		uint32 m_frameIndex{0u};
		uint32 m_imageIndex{0u};
	};
}
