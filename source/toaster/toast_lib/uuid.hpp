#pragma once

#include "system_types.h"

#include <xhash>

namespace toaster
{
	// No need for id validation, as there is a 1 / 18,446,744,073,709,551,615 (1 in 18.5 quintillion) chance of a collision
	class UUID
	{
	public:
		UUID();

		constexpr UUID(uint64 p_uuid) : m_uuid(p_uuid)
		{
		}

		operator uint64() const;

	private:
		uint64 m_uuid{0u};
	};
}

namespace std
{
	template<>
	struct hash<toaster::UUID>
	{
		size_t operator()(const toaster::UUID &p_uuid) const noexcept
		{
			return hash<uint64>{}(p_uuid);
		}
	};
}
