#include "storage_buffer.hpp"

#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	StorageBuffer::StorageBuffer(uint32_t size, const StorageBufferSpecInfo &specification)
		: m_specInfo(specification)
	{
		m_bufferDesc = nvrhi::BufferDesc().setByteSize(size).setCanHaveRawViews(true).setCanHaveUAVs(true).setInitialState(nvrhi::ResourceStates::UnorderedAccess).
				setKeepInitialState(true) // enable fully automatic state tracking
				.setCpuAccess(m_specInfo.GPUOnly ? nvrhi::CpuAccessMode::None : nvrhi::CpuAccessMode::Write).setDebugName(m_specInfo.DebugName);

		invalidate();
	}

	void StorageBuffer::invalidate()
	{
		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_handle                   = device->createBuffer(m_bufferDesc);

		if (!m_specInfo.GPUOnly)
			m_localStorage.realloc(m_bufferDesc.byteSize);
	}

	void StorageBuffer::setData(Buffer buffer, uint32_t offset)
	{
		m_localStorage.write(buffer);

		RefPtr<StorageBuffer> instance = this;
		Renderer::submit([instance, offset]() mutable
		{
			instance->_setData(instance->m_localStorage, offset);
		});
	}

	void StorageBuffer::setData(const void *data, uint32_t size, uint32_t offset)
	{
		setData(Buffer(data, size), offset);
	}

	void StorageBuffer::_setData(Buffer buffer, uint32_t offset)
	{
		if (buffer.size == 0)
			return;

		if (!m_commandList)
			m_commandList = make_reference<CommandBuffer>(1, "StorageBuffer");

		m_commandList->_beginRecording();
		m_commandList->getActiveCommandBuffer()->writeBuffer(m_handle, buffer.data, buffer.size, offset);
		m_commandList->_endRecording();
		m_commandList->_submit();
	}

	void StorageBuffer::_setData(const void *data, uint32_t size, uint32_t offset)
	{
		_setData(Buffer(data, size), offset);
	}

	void StorageBuffer::resize(uint32_t size)
	{
		m_bufferDesc.setByteSize(size);
		invalidate();
	}
}
