#include "vertex_buffer.hpp"

#include "platform/application.hpp"

namespace tst::gpu
{
	VertexBuffer::VertexBuffer(const Buffer &buffer) : m_size(buffer.size)
	{
		auto vertex_buffer_description             = nvrhi::BufferDesc();
		vertex_buffer_description.byteSize         = buffer.size;
		vertex_buffer_description.isVertexBuffer   = true;
		vertex_buffer_description.debugName        = "Vertex Buffer";
		vertex_buffer_description.initialState     = nvrhi::ResourceStates::VertexBuffer;
		vertex_buffer_description.keepInitialState = true;

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_bufferHandle             = device->createBuffer(vertex_buffer_description);

		m_commandBuffer = make_reference<CommandBuffer>(1, "Vertex Buffer");
		m_commandBuffer->_beginRecording();
		m_commandBuffer->getActiveCommandBuffer()->writeBuffer(m_bufferHandle, buffer.data, buffer.size);
		m_commandBuffer->_endRecording();
		m_commandBuffer->_submit();
	}

	VertexBuffer::VertexBuffer(uint64 size) : m_size(size)
	{
		auto vertex_buffer_description             = nvrhi::BufferDesc();
		vertex_buffer_description.byteSize         = size;
		vertex_buffer_description.isVertexBuffer   = true;
		vertex_buffer_description.debugName        = "Vertex Buffer";
		vertex_buffer_description.initialState     = nvrhi::ResourceStates::VertexBuffer;
		vertex_buffer_description.cpuAccess        = nvrhi::CpuAccessMode::Write;
		vertex_buffer_description.keepInitialState = true;

		nvrhi::DeviceHandle device = Application::getGraphicsDevice();
		m_bufferHandle             = device->createBuffer(vertex_buffer_description);
	}

	void VertexBuffer::setData(const Buffer &buffer, uint64 offset)
	{
		if (buffer.size == 0)
			return;

		if (!m_commandBuffer)
			m_commandBuffer = make_reference<CommandBuffer>(1, "Vertex Buffer");

		auto  device        = Application::getGraphicsDevice();
		void *mapped_buffer = device->mapBuffer(m_bufferHandle, nvrhi::CpuAccessMode::Write);
		std::memcpy(mapped_buffer, static_cast<uint8 *>(buffer.data) + offset, buffer.size);
		device->unmapBuffer(m_bufferHandle);
	}

	void VertexBuffer::setData(const void *data, uint64 size, uint64 offset)
	{
		setData(Buffer(data, size), offset);
	}

	void VertexBuffer::_setData(const Buffer &buffer, uint64 offset)
	{
		if (buffer.size == 0)
			return;

		if (!m_commandBuffer)
			m_commandBuffer = make_reference<CommandBuffer>(1, "Vertex Buffer");

		m_commandBuffer->_beginRecording();
		m_commandBuffer->getActiveCommandBuffer()->writeBuffer(m_bufferHandle, buffer.data, buffer.size);
		m_commandBuffer->_endRecording();
		m_commandBuffer->_submit();
	}

	void VertexBuffer::_setData(const void *data, uint64 size, uint64 offset)
	{
		_setData(Buffer(data, size), offset);
	}
}
