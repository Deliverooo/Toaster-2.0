#pragma once

#include <functional>
#include <vulkan/vulkan_raii.hpp>

#include "vk_command_buffer.hpp"
#include "vk_common.hpp"
#include "toast_lib/system_types.h"

namespace toaster::gpu
{
	// This is after presenting, so the layout becomes undefined
	constexpr ImageLayoutInfo c_swapchainEndFrameLayoutInfo{vk::ImageLayout::eUndefined, vk::AccessFlagBits2::eNone, vk::PipelineStageFlagBits2::eColorAttachmentOutput};
	constexpr ImageLayoutInfo c_swapchainEndFrameDepthLayoutInfo{
		vk::ImageLayout::eUndefined,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests
	};

	constexpr ImageLayoutInfo c_swapchainBeginFrameLayoutInfo{
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	};

	constexpr ImageLayoutInfo c_swapchainPresentSrcLayoutInfo{
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eNone,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput
	};

	class TST_GPU_API VKSwapchain
	{
		TST_GPU_OBJECT
	public:
		using BeginFrameCB              = void(*)(void *, uint32);
		using ResizeCB                  = void(*)(void *, tsm::uint2);
		using HandleMinimisationCB      = std::function<void()>;
		using GetWindowBackBufferSizeCB = std::function<std::pair<uint32, uint32>()>;

		VKSwapchain(VKGPUContext& p_gpu_ctx, vk::SurfaceKHR *p_surface);
		~VKSwapchain();

		auto beginFrame() -> void;
		auto endFrame() -> void;

		[[nodiscard]] auto getFrameIndex() const -> uint32;
		[[nodiscard]] auto getImageIndex() const -> uint32;

		[[nodiscard]] auto getImage(uint32 p_index) -> vk::Image &;
		[[nodiscard]] auto getImageView(uint32 p_index) -> vk::ImageView &;

		[[nodiscard]] auto getCurrentImage() -> vk::Image &;
		[[nodiscard]] auto getCurrentImageView() -> vk::ImageView &;

		[[nodiscard]] auto getDepthImage() -> vk::raii::Image &;
		[[nodiscard]] auto getDepthImageView() -> vk::raii::ImageView &;
		[[nodiscard]] auto getDepthImageMemory() -> vk::raii::DeviceMemory &;

		[[nodiscard]] auto getCommandBuffer(uint32 p_frame_index) -> VKCommandBuffer &;
		[[nodiscard]] auto getCurrentCommandBuffer() -> VKCommandBuffer &;

		[[nodiscard]] auto getExtent() const -> vk::Extent2D;
		[[nodiscard]] auto getAspectRatio() const -> float32;
		[[nodiscard]] auto getSurfaceFormat() const -> vk::SurfaceFormatKHR;
		[[nodiscard]] auto getDepthFormat() const -> vk::Format;

		[[nodiscard]] auto getMinImageCount() const -> uint32;
		[[nodiscard]] auto getImageCount() const -> uint32;

		// Only use within the windowing API. E.g. when you set the glfwFramebufferResize callback
		auto setFramebufferResized(bool p_resized) -> void;

		auto setUserDataPointer(void *p_user_data) -> void;
		auto setResizeUserDataPointer(void *p_user_data) -> void;

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

		vk::SurfaceKHR *m_windowSurface{nullptr};

		vk::raii::SwapchainKHR     m_swapchain{nullptr};
		std::vector<vk::Image>     m_swapchainImages;
		std::vector<vk::ImageView> m_swapchainImageViews;
		vk::SurfaceFormatKHR       m_swapchainSurfaceFormat;
		vk::Extent2D               m_swapchainExtent;
		uint32                     m_minImageCount{0u};

		std::vector<vk::raii::Semaphore> m_imageAvailableSemaphores;
		std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;

		std::vector<VKCommandBuffer> m_commandBuffers;

		vk::raii::Image        m_depthImage{nullptr};
		vk::raii::DeviceMemory m_depthImageMemory{nullptr};
		vk::raii::ImageView    m_depthImageView{nullptr};

		void *       m_userData{nullptr};
		BeginFrameCB m_beginFrameCallback{nullptr};

		void *   m_resizeData{nullptr};
		ResizeCB m_resizeCallback{nullptr};

		HandleMinimisationCB      m_handleMinimisationCallback{nullptr};
		GetWindowBackBufferSizeCB m_getWindowBackBufferSizeCallback{nullptr};

		uint32 m_frameIndex{0u};
		uint32 m_imageIndex{0u};

		bool m_framebufferResized{false};
	};
}
