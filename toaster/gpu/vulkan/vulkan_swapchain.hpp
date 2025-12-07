/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#pragma once

#include <queue>

#include "core_typedefs.hpp"

#include <vulkan/vulkan.hpp>
#include <nvrhi/nvrhi.h>

namespace tst
{
	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(vk::SurfaceKHR surface);

		bool create(uint32 width, uint32 height);
		void destroy();

		void onResize(uint32 width, uint32 height);
		void resize();

		bool beginFrame();
		void present();

		nvrhi::IFramebuffer *getCurrentFramebuffer();
		nvrhi::IFramebuffer *getFramebuffer(uint32 index);

		nvrhi::ITexture *getCurrentBackBuffer();
		nvrhi::ITexture *getBackBuffer(uint32 index);

		[[nodiscard]] uint32 getCurrentBackBufferIndex() const;
		uint32               getBackBufferCount();

		[[nodiscard]] vk::Semaphore getAcquiredImageSemaphore() const { return m_acquiredSemaphore; }

		void backBufferResizing();
		void backBufferResized();

	private:
		vk::SurfaceKHR m_surface = nullptr;
		uint32         m_width   = 0u;
		uint32         m_height  = 0u;

		std::array<vk::Semaphore, 3> m_acquireSemaphores;
		std::array<vk::Semaphore, 3> m_presentSemaphores;
		vk::Semaphore                m_acquiredSemaphore;

		std::vector<nvrhi::FramebufferHandle> m_swapchainFramebuffers;

		vk::SurfaceFormatKHR m_swapchainFormat;
		vk::SwapchainKHR     m_swapchain;

		uint32 m_acquireSemaphoreIndex = 0u;
		uint32 m_presentSemaphoreIndex = 0u;

		struct SwapchainImage
		{
			vk::Image            image;
			nvrhi::TextureHandle handle;
		};

		std::vector<SwapchainImage> m_swapchainImages;
		uint32                      m_swapchainIndex = UINT32_MAX;

		std::queue<nvrhi::EventQueryHandle>  m_framesInFlight;
		std::vector<nvrhi::EventQueryHandle> m_queryPool;
	};
}
