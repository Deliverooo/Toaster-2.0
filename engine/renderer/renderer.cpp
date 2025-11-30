#include "renderer.hpp"

#include "gpu/vulkan/vulkan_swapchain.hpp"
#include "platform/application.hpp"

namespace tst
{
	struct RendererData
	{
	};

	static RendererData *s_data = nullptr;

	constexpr uint32           s_renderCommandQueueCount = 2;
	RenderCommandQueue *       s_renderCommandQueue[s_renderCommandQueueCount];
	static std::atomic<uint32> s_renderCommandQueueSubmissionIndex = 0;

	EError Renderer::init()
	{
		s_data = tnew RendererData();

		s_renderCommandQueue[0] = tnew RenderCommandQueue();
		s_renderCommandQueue[1] = tnew RenderCommandQueue();

		return EError::eOk;
	}

	void Renderer::terminate()
	{
		Application::getDeviceManager()->getDevice()->waitForIdle();

		delete s_data;

		delete s_renderCommandQueue[0];
		delete s_renderCommandQueue[1];
	}

	void Renderer::waitAndRender(RenderThread *renderThread)
	{
		renderThread->waitAndSet(RenderThread::EState::eKick, RenderThread::EState::eBusy);

		s_renderCommandQueue[getRenderQueueIndex()]->executeCommands();

		renderThread->set(RenderThread::EState::eIdle);
	}

	void Renderer::swapQueues()
	{
		s_renderCommandQueueSubmissionIndex = (s_renderCommandQueueSubmissionIndex + 1) % s_renderCommandQueueCount;
	}

	void Renderer::renderThreadFunc(RenderThread *renderThread)
	{
		while (renderThread->isRunning())
		{
			waitAndRender(renderThread);
		}
	}

	uint32 Renderer::getRenderQueueIndex()
	{
		return (s_renderCommandQueueSubmissionIndex + 1) % s_renderCommandQueueCount;
	}

	uint32 Renderer::getRenderQueueSubmissionIndex()
	{
		return s_renderCommandQueueSubmissionIndex;
	}

	void Renderer::beginFrame()
	{
	}

	void Renderer::endFrame()
	{
	}

	uint32_t Renderer::getCurrentFrameIndex()
	{
		return Application::getInstance().getCurrentFrameIndex();
	}

	uint32_t Renderer::_getCurrentFrameIndex()
	{
		return Application::getInstance().getMainWindow().getSwapchain()->getCurrentBackBufferIndex();
	}

	RenderCommandQueue &Renderer::_getRenderCommandQueue()
	{
		return *s_renderCommandQueue[s_renderCommandQueueSubmissionIndex];
	}
}
