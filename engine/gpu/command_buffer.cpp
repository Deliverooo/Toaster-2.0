#include "command_buffer.hpp"

#include <mutex>
#include <utility>

#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst::gpu
{
	static std::mutex s_graphicsLock;

	CommandBuffer::CommandBuffer(uint32 count, String debug_name) : m_debugName(std::move(debug_name))
	{
		if (count == 0)
		{
			count = Renderer::getConfigInfo().maxFramesInFlight;
		}

		auto device = Application::getGraphicsDevice();
		for (uint32 i = 0; i < count; ++i)
		{
			m_commandLists.push_back(device->createCommandList());
		}
	}

	void CommandBuffer::beginRecording()
	{
		Renderer::submit([this]() mutable { _beginRecording(); });
	}

	void CommandBuffer::endRecording()
	{
		Renderer::submit([this]() mutable { _endRecording(); });
	}

	void CommandBuffer::submit()
	{
		Renderer::submit([this]() mutable { _submit(); });
	}

	void CommandBuffer::_beginRecording()
	{
		uint32 command_buffer_index = Renderer::_getCurrentFrameIndex();
		command_buffer_index %= m_commandLists.size();

		m_activeCommandBuffer = m_commandLists[command_buffer_index];
		m_activeCommandBuffer->open();
	}

	void CommandBuffer::_endRecording()
	{
		m_activeCommandBuffer->close();
		m_activeCommandBuffer = nullptr;
	}

	void CommandBuffer::_submit()
	{
		const auto device = Application::getGraphicsDevice();

		uint32 command_buffer_index = Renderer::_getCurrentFrameIndex();
		command_buffer_index %= m_commandLists.size();

		lockGraphicsQueue();
		device->executeCommandList(m_commandLists[command_buffer_index]);
		unlockGraphicsQueue();
	}

	void CommandBuffer::lockGraphicsQueue()
	{
		s_graphicsLock.lock();
	}

	void CommandBuffer::unlockGraphicsQueue()
	{
		s_graphicsLock.unlock();
	}
}
