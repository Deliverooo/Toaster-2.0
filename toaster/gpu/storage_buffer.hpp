#pragma once

#include "buffer.hpp"
#include "command_buffer.hpp"

namespace tst
{
	struct StorageBufferSpecInfo
	{
		bool        GPUOnly   = true;
		std::string DebugName = "StorageBuffer";
	};

	class StorageBuffer : public RefCounted
	{
	public:
		StorageBuffer(uint32_t size, const StorageBufferSpecInfo &specification);
		~StorageBuffer() = default;

		void invalidate();

		nvrhi::BufferHandle getHandle() const { return m_handle; }

		void setData(Buffer buffer, uint32_t offset = 0);
		void setData(const void *data, uint32_t size, uint32_t offset = 0);
		void _setData(Buffer buffer, uint32_t offset = 0);
		void _setData(const void *data, uint32_t size, uint32_t offset = 0);
		void resize(uint32_t size);

	private:
		StorageBufferSpecInfo m_specInfo;

		nvrhi::BufferHandle m_handle;
		nvrhi::BufferDesc   m_bufferDesc;

		RefPtr<CommandBuffer> m_commandList;

		Buffer m_localStorage;
	};
}
