#pragma once

#include <queue>
#include <nvrhi/nvrhi.h>
#include <nvrhi/vulkan.h>
#include <vulkan/vulkan.hpp>

#include "gpu_context.hpp"
#include "system_types.h"

namespace toaster::gpu
{
	class GPUContext;

	class Swapchain
	{
	public:
		Swapchain(GPUContext *p_ctx, vk::SurfaceKHR p_window_surface);

		void create(uint32 p_width, uint32 p_height);
		void destroy();

		void onResize(uint32 p_width, uint32 p_height);
		void resize();

		vk::Result beginFrame();
		void       present();

		[[nodiscard]] uint32 getCurrentFrameIndex() const;

		nvrhi::ITexture *getImage(uint32 p_frame_index);
		nvrhi::ITexture *getCurrentImage();

		nvrhi::IFramebuffer *getFramebuffer(uint32 p_frame_index);
		nvrhi::IFramebuffer *getCurrentFramebuffer();

	private:
		void _createSwapchainImages();
		void _createSwapchainFramebuffers();

		GPUContext *m_gpuContext{nullptr};

		vk::SurfaceKHR m_surface{nullptr};
		uint32         m_width{0u};
		uint32         m_height{0u};

		std::array<vk::Semaphore, 3> m_acquireSemaphores;
		std::vector<vk::Semaphore>   m_presentSemaphores; // One per swapchain image
		vk::Semaphore                m_acquiredSemaphore;

		std::vector<nvrhi::FramebufferHandle> m_swapchainFramebuffers;

		vk::SurfaceFormatKHR m_swapchainFormat;
		vk::SwapchainKHR     m_swapchain;

		uint32 m_acquireSemaphoreIndex{0u};

		struct SwapchainImage
		{
			vk::Image            image;
			nvrhi::TextureHandle handle;
		};

		std::vector<SwapchainImage> m_swapchainImages;
		uint32                      m_swapchainIndex{UINT32_MAX};

		std::queue<nvrhi::EventQueryHandle>  m_framesInFlight;
		std::vector<nvrhi::EventQueryHandle> m_queryPool;
	};
}
