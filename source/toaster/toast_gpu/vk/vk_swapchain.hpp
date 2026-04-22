#pragma once

#include <functional>
#include <vulkan/vulkan_raii.hpp>

#include "vk_command_buffer.hpp"
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
		auto getContext() const -> VKGPUContext *;

		auto beginFrame() -> void;
		auto endFrame() -> void;

		[[nodiscard]] auto getFrameIndex() const -> uint32;
		[[nodiscard]] auto getImageIndex() const -> uint32;

		[[nodiscard]] auto getImage(uint32 p_index) -> vk::Image &;
		[[nodiscard]] auto getImageView(uint32 p_index) -> vk::raii::ImageView &;

		[[nodiscard]] auto getCurrentImage() -> vk::Image &;
		[[nodiscard]] auto getCurrentImageView() -> vk::raii::ImageView &;

		[[nodiscard]] auto getDepthImage() -> vk::raii::Image &;
		[[nodiscard]] auto getDepthImageView() -> vk::raii::ImageView &;
		[[nodiscard]] auto getDepthImageMemory() -> vk::raii::DeviceMemory &;

		[[nodiscard]] auto getCommandBuffer(uint32 p_frame_index) -> vk::raii::CommandBuffer &;
		[[nodiscard]] auto getCurrentCommandBuffer() -> vk::raii::CommandBuffer &;

		[[nodiscard]] auto getExtent() const -> vk::Extent2D;
		[[nodiscard]] auto getSurfaceFormat() const -> vk::SurfaceFormatKHR;
		[[nodiscard]] auto getDepthFormat() const -> vk::Format;

		[[nodiscard]] auto getMinImageCount() const -> uint32;
		[[nodiscard]] auto getImageCount() const -> uint32;

		// Only use within the windowing API. E.g. when you set the glfwFramebufferResize callback
		auto setFramebufferResized(bool p_resized) -> void;

		auto addResizeCallback(const ResizeCB &p_resize_cb) -> void;

	private:
		auto _createImageViews() -> void;
		auto _createSyncObjects() -> void;
		auto _createCommandBuffers() -> void;
		auto _createDepthResources() -> void;

		auto _create() -> void;
		auto _recreateSwapchain() -> void;

		[[nodiscard]] auto _chooseSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_available_formats) const -> vk::SurfaceFormatKHR;
		[[nodiscard]] auto _chooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &p_available_present_modes) const -> vk::PresentModeKHR;
		[[nodiscard]] auto _chooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const -> vk::Extent2D;
		[[nodiscard]] auto _chooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surface_capabilities) const -> uint32;

		VKGPUContext *m_ctx{nullptr};
		GLFWwindow *  m_window{nullptr};

		vk::raii::SwapchainKHR           m_swapchain{nullptr};
		std::vector<vk::Image>           m_swapchainImages;
		std::vector<vk::raii::ImageView> m_swapchainImageViews;
		vk::SurfaceFormatKHR             m_swapchainSurfaceFormat;
		vk::Extent2D                     m_swapchainExtent;
		uint32                           m_minImageCount{0u};

		std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
		VKCommandBufferPFF               m_commandBuffers;

		vk::raii::Image        m_depthImage{nullptr};
		vk::raii::DeviceMemory m_depthImageMemory{nullptr};
		vk::raii::ImageView    m_depthImageView{nullptr};

		std::vector<ResizeCB> m_resizeCallbacks;

		uint32 m_frameIndex{0u};
		uint32 m_imageIndex{0u};

		bool m_framebufferResized{false};
	};
}
