#pragma once

#include "toast_gpu.hpp"
#include "toast_lib/command_buffer.hpp"

#include <vector>

namespace toaster::gpu
{
	class TST_GPU_API DeletionQueue
	{
	public:
		DeletionQueue(uint32 p_queue_count)
		{
			m_pendingDeletions.resize(p_queue_count);
		}

		~DeletionQueue()
		{
			m_pendingDeletions.clear();
		}

		DeletionQueue(const DeletionQueue &)            = delete;
		DeletionQueue(DeletionQueue &&)                 = delete;
		DeletionQueue &operator=(const DeletionQueue &) = delete;
		DeletionQueue &operator=(DeletionQueue &&)      = delete;

		auto setQueueIndex(uint32 p_index) -> void { m_queueIndex = p_index; }

		auto execute() -> void { m_pendingDeletions[m_queueIndex].execute(); }

		auto executeAll() -> void
		{
			for (auto &queue: m_pendingDeletions)
				queue.execute();
		}

		template<typename TFunc> requires  std::is_invocable_v<TFunc> && std::is_nothrow_invocable_v<TFunc>
		auto submit(TFunc &&p_func) -> void
		{
			m_pendingDeletions[m_queueIndex].enqueue(std::forward<TFunc>(p_func));
		}

	private:
		std::vector<CommandBuffer> m_pendingDeletions;
		uint32                     m_queueIndex{0u};
	};
}
