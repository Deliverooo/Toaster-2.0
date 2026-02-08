/*!
 * @file buffer.hpp
 */
#pragma once

#include <memory>
#include "system_types.h"
#include "toast_assert.h"

namespace toaster
{
	class Buffer
	{
	public:
		Buffer() = default;

		Buffer(const void *p_data, const uint64 p_size)
			: m_data(const_cast<void *>(p_data)), m_size(p_size)
		{
		}

		static Buffer copy(const Buffer &p_other)
		{
			Buffer buffer;
			buffer.allocate(p_other.m_size);
			memcpy(buffer.m_data, p_other.m_data, p_other.m_size);
			return buffer;
		}

		static Buffer copy(const void *p_data, uint64 p_size)
		{
			Buffer buffer;
			buffer.allocate(p_size);
			if (p_size)
				memcpy(buffer.m_data, p_data, p_size);
			return buffer;
		}

		void allocate(const uint64 p_size)
		{
			delete[] static_cast<uint8 *>(m_data);
			m_data = nullptr;
			m_size = p_size;

			if (p_size == 0)
				return;

			m_data = new uint8[p_size];
		}

		void reallocate(const uint64 p_size)
		{
			release();
			allocate(p_size);
		}

		void release()
		{
			delete[] static_cast<uint8 *>(m_data);
			m_data = nullptr;
			m_size = 0;
		}

		void zeroInitialize()
		{
			if (m_data)
				memset(m_data, 0, m_size);
		}

		template<typename Type>
		Type &read(const uint64 p_offset = 0u)
		{
			return *static_cast<Type *>(static_cast<uint8 *>(m_data) + p_offset);
		}

		template<typename Type>
		const Type &read(uint64 p_offset = 0u) const
		{
			return *static_cast<Type *>(static_cast<uint8 *>(m_data) + p_offset);
		}

		[[nodiscard]] uint8 *readBytes(uint64 p_size, uint64 p_offset) const
		{
			TST_ASSERT_MSG(p_offset + p_size <= m_size, "Buffer overflow!");
			const auto buffer = new uint8[p_size];
			std::memcpy(buffer, static_cast<uint8 *>(m_data) + p_offset, p_size);
			return buffer;
		}

		void write(Buffer p_buffer, uint64 p_offset = 0u)
		{
			TST_ASSERT_MSG(p_offset + p_buffer.m_size <= m_size, "Buffer overflow!");
			std::memcpy(static_cast<uint8 *>(m_data) + p_offset, p_buffer.m_data, p_buffer.m_size);
		}

		void write(const void *p_data, uint64 p_size, uint64 p_offset = 0u)
		{
			write(Buffer(p_data, p_size), p_offset);
		}

		operator bool() const
		{
			return static_cast<bool>(m_data);
		}

		uint8 &operator[](int p_index)
		{
			return static_cast<uint8 *>(m_data)[p_index];
		}

		uint8 operator[](int p_index) const
		{
			return static_cast<uint8 *>(m_data)[p_index];
		}

		template<typename Type>
		Type *as() const
		{
			return static_cast<Type *>(m_data);
		}

		[[nodiscard]] uint64 size() const { return m_size; }
		[[nodiscard]] void * data() const { return m_data; }

	private:
		void * m_data{nullptr};
		uint64 m_size{0u};
	};
}
