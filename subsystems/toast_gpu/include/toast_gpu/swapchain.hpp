#pragma once

#include "gpu_common.hpp"
#include "device.hpp"
#include "toast_math/math_vector.hpp"

namespace toaster::gpu
{
	class TST_GPU_API Swapchain
	{
		TST_REGISTER_DEPENDENCY(Device, Device, device)

	public:
		Swapchain(Device &p_device, vk::SurfaceKHR p_surface, uint32 p_max_frames_in_flight, uint32 p_initial_width, uint32 p_initial_height);
		~Swapchain();

		// This is the order you should call these in a frame loop
		auto acquireImage(vk::Semaphore p_signal_semaphore) -> void;
		auto beginFrame(CommandList &p_cmd) -> void;
		auto endFrame(CommandList &p_cmd) -> void;
		auto getSignalSemaphoreInfo() const -> vk::SemaphoreSubmitInfo;
		auto present() -> void;

		auto onResize(uint32 p_width, uint32 p_height) -> void;

		auto getExtent() const -> tsm::Extent2D { return m_swapchainExtent; }
		auto getMinImageCount() const -> uint32 { return m_minImageCount; }
		auto getMaxFramesInFlight() const -> uint32 { return m_maxFramesInFlight; }
		auto getSurfaceFormat() const -> vk::SurfaceFormatKHR { return m_swapchainSurfaceFormat; }
		auto getDepthFormat() const -> vk::Format { return m_depthFormat; }

		auto getImageIndex() const -> uint32 { return m_imageIndex; }

		auto getCurrentImage() const -> vk::Image { return m_images[m_imageIndex]; }
		auto getCurrentColourTexture() const -> TextureHandle { return m_colourTextures[m_imageIndex]; }

		auto getDepthTexture() const -> TextureHandle { return m_depthTexture; }

		auto getColourAttachmentInfo(const vk::ClearColorValue &p_clear_colour = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f}) const -> vk::RenderingAttachmentInfo;
		auto getDepthAttachmentInfo(vk::ClearDepthStencilValue p_clear_value = vk::ClearDepthStencilValue{1.0f, 0u}) const -> vk::RenderingAttachmentInfo;

	private:
		auto _create(uint32 p_width, uint32 p_height) -> void;

		tsm::Extent2D           m_swapchainExtent{};
		uint32               m_minImageCount{0u};
		uint32               m_maxFramesInFlight{0u};
		vk::SurfaceFormatKHR m_swapchainSurfaceFormat;
		vk::Format           m_depthFormat{vk::Format::eUndefined};

		vk::SwapchainKHR m_swapchain{nullptr};
		vk::SurfaceKHR   m_windowSurface{nullptr};

		// PFF = Per frame in flight. PSI = Per swapchain image
		std::vector<vk::Image>     m_images;         // PSI
		std::vector<TextureHandle> m_colourTextures; // PSI

		TextureHandle m_depthTexture;

		std::vector<vk::Semaphore> m_renderFinishedSemaphores; // PSI

		uint32 m_imageIndex{0u};
	};
}
