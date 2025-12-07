#include "index_buffer.hpp"
#include "platform/application.hpp"

namespace tst
{
	IndexBuffer::IndexBuffer(const Buffer &buffer) : m_size(buffer.size)
	{
		auto index_buffer_description             = nvrhi::BufferDesc();
		index_buffer_description.byteSize         = buffer.size;
		index_buffer_description.isVertexBuffer   = true;
		index_buffer_description.debugName        = "Index Buffer";
		index_buffer_description.initialState     = nvrhi::ResourceStates::IndexBuffer;
		index_buffer_description.keepInitialState = true;

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_bufferHandle             = device->createBuffer(index_buffer_description);

		m_commandBuffer = make_reference<CommandBuffer>(1, "Index Buffer");
		m_commandBuffer->_beginRecording();
		m_commandBuffer->getActiveCommandBuffer()->writeBuffer(m_bufferHandle, buffer.data, buffer.size);
		m_commandBuffer->_endRecording();
		m_commandBuffer->_submit();
	}

	IndexBuffer::IndexBuffer(uint64 size) : m_size(size)
	{
		auto index_buffer_description             = nvrhi::BufferDesc();
		index_buffer_description.byteSize         = size;
		index_buffer_description.isVertexBuffer   = true;
		index_buffer_description.debugName        = "Index Buffer";
		index_buffer_description.initialState     = nvrhi::ResourceStates::IndexBuffer;
		index_buffer_description.cpuAccess        = nvrhi::CpuAccessMode::Write;
		index_buffer_description.keepInitialState = true;

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_bufferHandle             = device->createBuffer(index_buffer_description);
	}

	void IndexBuffer::setData(const void *data, uint64 size, uint64 offset)
	{
		// todo
	}
}
