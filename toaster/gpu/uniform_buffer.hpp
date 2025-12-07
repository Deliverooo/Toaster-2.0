#pragma once

#include "buffer.hpp"
#include "gpu/command_buffer.hpp"

namespace tst
{
	class UniformBuffer : public RefCounted
	{
	public:
		nvrhi::BufferHandle GetHandle() const { return m_Handle; }

		void SetData(Buffer buffer, uint64_t offset = 0);
		void SetData(const void *data, uint64_t size, uint64_t offset = 0);

		void RT_SetData(Buffer buffer, uint64_t offset = 0);
		void RT_SetData(const void *data, uint64_t size, uint64_t offset = 0);

		UniformBuffer(uint64_t size, std::string_view debugName = "UniformBuffer");
		virtual ~UniformBuffer() = default;

	private:
		uint64_t    m_Size = 0;
		std::string m_DebugName;

		nvrhi::BufferHandle m_Handle;

		RefPtr<CommandBuffer> m_CommandList;

		Buffer m_LocalData;
	};
}
