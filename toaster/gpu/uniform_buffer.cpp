#include "uniform_buffer.hpp"

#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	UniformBuffer::UniformBuffer(uint64_t size, std::string_view debugName)
		: m_Size(size), m_DebugName(debugName)
	{
		m_LocalData.alloc(size);

		auto bufferDesc = nvrhi::BufferDesc().setByteSize(size).setIsConstantBuffer(true).setCpuAccess(nvrhi::CpuAccessMode::Write).
				setInitialState(nvrhi::ResourceStates::ConstantBuffer).setKeepInitialState(true) // enable fully automatic state tracking
				.setDebugName(m_DebugName.c_str());

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_Handle                   = device->createBuffer(bufferDesc);
	}

	void UniformBuffer::SetData(const void *data, uint64_t size, uint64_t offset)
	{
		SetData(Buffer(data, size), offset);
	}

	void UniformBuffer::SetData(Buffer buffer, uint64_t offset)
	{
		m_LocalData.write(buffer);

		RefPtr<UniformBuffer> instance = this;
		Renderer::submit([instance, data = m_LocalData, offset]() mutable { instance->RT_SetData(data, offset); });
	}

	void UniformBuffer::RT_SetData(const void *data, uint64_t size, uint64_t offset)
	{
		RT_SetData(Buffer(data, size), offset);
	}

	void UniformBuffer::RT_SetData(Buffer buffer, uint64_t offset)
	{
		if (buffer.size == 0)
			return;

		if (!m_CommandList)
			m_CommandList = make_reference<CommandBuffer>(1, "StorageBuffer");

		m_CommandList->_beginRecording();
		m_CommandList->getActiveCommandBuffer()->writeBuffer(m_Handle, buffer.data, buffer.size, offset);
		m_CommandList->_endRecording();
		m_CommandList->_submit();
	}
}
