#pragma once

#include <functional>
#include <vulkan/vulkan_raii.hpp>

#include "vk_command_buffer.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	class VKLogicalDevice;

	class TST_GPU_API VKSwapchain
	{
	public:
		using BeginFrameCB              = std::function<void(VKLogicalDevice *, uint32)>;
		using ResizeCB                  = std::function<void(uint32, uint32)>;
		using HandleMinimisationCB      = std::function<void()>;
		using GetWindowBackBufferSizeCB = std::function<std::pair<uint32, uint32>()>;

		VKSwapchain(VKLogicalDevice *p_dev, vk::SurfaceKHR *p_surface);
		[[nodiscard]] auto getDevice() const -> VKLogicalDevice *;

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

		auto setBeginFrameCallback(const BeginFrameCB &p_begin_frame_cb) -> void;
		auto setResizeCallback(const ResizeCB &p_resize_cb) -> void;
		auto setHandleMinimisationCallback(const HandleMinimisationCB &p_handle_minimisation_callback) -> void;
		auto setGetWindowBackBufferSizeCallback(const GetWindowBackBufferSizeCB &p_get_window_back_buffer_size_callback) -> void;

	private:
		auto _createImageViews() -> void;
		auto _createSyncObjects() -> void;
		auto _createDepthResources() -> void;

		auto _create() -> void;
		auto _recreateSwapchain() -> void;

		VKLogicalDevice *m_device{nullptr};

		vk::SurfaceKHR *m_windowSurface{nullptr};

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

		BeginFrameCB              m_beginFrameCallback{nullptr};
		ResizeCB                  m_resizeCallback{nullptr};
		HandleMinimisationCB      m_handleMinimisationCallback{nullptr};
		GetWindowBackBufferSizeCB m_getWindowBackBufferSizeCallback{nullptr};

		uint32 m_frameIndex{0u};
		uint32 m_imageIndex{0u};

		bool m_framebufferResized{false};
	};
}
