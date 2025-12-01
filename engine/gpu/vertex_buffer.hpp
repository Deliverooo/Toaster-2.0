#pragma once

#include "../core/memory/buffer.hpp"
#include "command_buffer.hpp"

namespace tst::gpu
{
	class VertexBuffer : public RefCounted
	{
	public:
		VertexBuffer(const Buffer &buffer);
		VertexBuffer(uint64 size);

		void setData(const Buffer &buffer, uint64 offset);
		void setData(const void *data, uint64 size, uint64 offset);

		void _setData(const Buffer &buffer, uint64 offset);
		void _setData(const void *data, uint64 size, uint64 offset);

		uint64              getSize() const { return m_size; }
		nvrhi::BufferHandle getBufferHandle() const { return m_bufferHandle; }

	private:
		nvrhi::BufferHandle   m_bufferHandle;
		RefPtr<CommandBuffer> m_commandBuffer; // Used to record the buffer commands to, such as _setData();

		uint64 m_size;
	};
}
