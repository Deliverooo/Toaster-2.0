#pragma once

#include <vector>

#include "handle.hpp"
#include "toast_assert.h"

namespace toaster
{
	// The best class ever... seriously
	template<typename Tag, typename TData> /*requires std::is_default_constructible_v<TData>*/
	struct Pool
	{
		Pool()  = default;
		~Pool() = default;

		explicit Pool(uint32 p_reserved_size)
		{
			reserve(p_reserved_size);
		}

		auto reserve(uint32 p_reserved_size) -> void
		{
			_data.reserve(p_reserved_size);
			m_magic.reserve(p_reserved_size);
			_alive.reserve(p_reserved_size);
		}

		auto create(const TData &p_object) -> Handle<Tag>
		{
			uint32 index{0u};
			if (!m_freeIndices.empty())
			{
				index = m_freeIndices.back();
				m_freeIndices.pop_back();
			}
			else
			{
				index = static_cast<uint32>(_data.size());
				_data.emplace_back();
				_alive.emplace_back();
				m_magic.emplace_back();
			}

			_alive[index] = true;
			_data[index]  = p_object;

			return Handle<Tag>{index, m_magic[index]};
		}

		auto destroy(Handle<Tag> p_handle) -> void
		{
			#ifndef TST_DISABLE_POOL_VALIDATION
			if (!isValid(p_handle))
				TST_PERMA_ASSERT_MSG(false, "Your handle is not valid... :(");
			#endif

			_alive[p_handle.id] = false;
			++m_magic[p_handle.id];

			m_freeIndices.push_back(p_handle.id);
		}

		auto isValid(Handle<Tag> p_handle) const -> bool
		{
			if (p_handle.id >= _data.size())
				return false;

			return _alive[p_handle.id] && (m_magic[p_handle.id] == p_handle.magic);
		}

		auto getData(Handle<Tag> p_handle) -> TData *
		{
			#ifndef TST_DISABLE_POOL_VALIDATION
			if (!isValid(p_handle))
				return nullptr;
			#endif
			return &_data[p_handle.id];
		}

		auto getSize() const -> uint32 { return _data.size(); }
		auto getCapacity() const -> uint32 { return _data.capacity(); }

		std::vector<TData> _data;  // Maybe you will want to access this directly, to prevent accidents, I prefix it with _
		std::vector<bool>  _alive; // I know this is a vector of bools, but it doesn't matter because I won't be taking the addresses
	private:
		std::vector<uint32> m_magic;
		std::vector<uint32> m_freeIndices;
	};
}
