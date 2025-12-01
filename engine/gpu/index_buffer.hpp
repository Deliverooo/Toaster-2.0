#pragma once

#include "../core/memory/buffer.hpp"
#include "command_buffer.hpp"
#include "memory/ref_ptr.hpp"

#include <nvrhi/nvrhi.h>

namespace tst::gpu
{
	class IndexBuffer : public RefCounted
	{
	public:
		IndexBuffer(const Buffer &buffer);
		IndexBuffer(uint64 size);

		void setData(const void *data, uint64 size, uint64 offset = 0);

		uint64              getSize() const { return m_size; }
		uint32              getCount() const { return static_cast<uint32>(m_size) / sizeof(uint32); }
		nvrhi::BufferHandle getBufferHandle() const { return m_bufferHandle; }

	private:
		nvrhi::BufferHandle   m_bufferHandle;
		RefPtr<CommandBuffer> m_commandBuffer; // Used to record the buffer commands to, such as _setData();

		uint64 m_size;
	};
}
