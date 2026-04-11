#pragma once

#include <functional>
#include <vulkan/vulkan_raii.hpp>
#include "toast_lib/system_types.h"

struct GLFWwindow;

namespace toaster::gpu
{
	class VKGPUContext;

	class VKSwapchain
	{
	public:
		using ResizeCB = std::function<void(uint32, uint32)>;

		VKSwapchain(VKGPUContext *p_ctx, GLFWwindow *p_window);
		VKGPUContext *getContext() const;

		void beginFrame();
		void endFrame();

		[[nodiscard]] uint32 getFrameIndex() const;
		[[nodiscard]] uint32 getImageIndex() const;

		[[nodiscard]] vk::Image &          getImage(uint32 p_index);
		[[nodiscard]] vk::raii::ImageView &getImageView(uint32 p_index);

		[[nodiscard]] vk::Image &          getCurrentImage();
		[[nodiscard]] vk::raii::ImageView &getCurrentImageView();

		[[nodiscard]] vk::raii::Image &       getDepthImage();
		[[nodiscard]] vk::raii::ImageView &   getDepthImageView();
		[[nodiscard]] vk::raii::DeviceMemory &getDepthImageMemory();

		[[nodiscard]] vk::raii::CommandBuffer &getCommandBuffer(uint32 p_frame_index);
		[[nodiscard]] vk::raii::CommandBuffer &getCurrentCommandBuffer();

		[[nodiscard]] vk::Extent2D         getExtent() const;
		[[nodiscard]] vk::SurfaceFormatKHR getSurfaceFormat() const;
		[[nodiscard]] vk::Format           getDepthFormat() const;

		[[nodiscard]] uint32 getMinImageCount() const;
		[[nodiscard]] uint32 getImageCount() const;

		// Only use within the windowing API. E.g. when you set the glfwFramebufferResize callback
		void setFramebufferResized(bool p_resized);

		void addResizeCallback(const ResizeCB &p_resize_cb);

	private:
		void _createImageViews();
		void _createSyncObjects();
		void _createCommandBuffers();
		void _createDepthResources();

		void _create();
		void _recreateSwapchain();

		VKGPUContext *m_ctx{nullptr};
		GLFWwindow *  m_window{nullptr};

		[[nodiscard]] vk::SurfaceFormatKHR _chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const;
		[[nodiscard]] vk::PresentModeKHR   _chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const;
		[[nodiscard]] vk::Extent2D         _chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const;
		[[nodiscard]] uint32               _chooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const;

		vk::raii::SwapchainKHR           m_swapchain{nullptr};
		std::vector<vk::Image>           m_swapchainImages;
		std::vector<vk::raii::ImageView> m_swapchainImageViews;
		vk::SurfaceFormatKHR             m_swapchainSurfaceFormat;
		vk::Extent2D                     m_swapchainExtent;
		uint32                           m_minImageCount{0u};

		std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
		std::vector<vk::raii::Fence>     m_inFlightFences;

		std::vector<vk::raii::CommandBuffer> m_commandBuffers;

		vk::raii::Image        m_depthImage{nullptr};
		vk::raii::DeviceMemory m_depthImageMemory{nullptr};
		vk::raii::ImageView    m_depthImageView{nullptr};

		std::vector<ResizeCB> m_resizeCallbacks;

		uint32 m_frameIndex{0u};
		uint32 m_imageIndex{0u};

		bool m_framebufferResized{false};
	};
}
