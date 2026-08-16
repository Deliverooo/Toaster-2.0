#pragma once

#include "system_types.h"
#include "toast_assert.h"

#include <vector>

namespace toaster
{
	// I think this may be a free list allocator...
	template<typename TSlotType = uint32> requires std::is_integral_v<TSlotType>
	struct FreelistAllocator
	{
		FreelistAllocator() = default;

		FreelistAllocator(TSlotType p_capacity) : m_capacity(p_capacity)
		{
		}

		auto allocSlot() -> TSlotType
		{
			if (!m_recycledSlots.empty())
			{
				TSlotType slot{m_recycledSlots.back()};
				m_recycledSlots.pop_back();
				return slot;
			}

			TST_PERMA_ASSERT_MSG(m_nextFreeIndex < m_capacity, "Allocation capacity reached!");
			return m_nextFreeIndex++;
		}

		auto freeSlot(TSlotType p_slot) -> void
		{
			m_recycledSlots.emplace_back(p_slot);
		}

	private:
		TSlotType              m_capacity;
		TSlotType              m_nextFreeIndex{0u};
		std::vector<TSlotType> m_recycledSlots;
	};
}
