#pragma once

#include <memory>
#include "toast_lib/system_types.h"
#include "toast_lib/toast_assert.h"

namespace toaster::gpu
{
	struct DataBuffer
	{
		void * data;
		uint64 size;

		DataBuffer() : data(nullptr), size(0u)
		{
		}

		DataBuffer(const void *p_data, uint64 p_size = 0u) : data(const_cast<void *>(p_data)), size(p_size)
		{
		}

		void alloc(uint64 p_size)
		{
			delete[]static_cast<uint8 *>(data);
			data = nullptr;

			if (p_size == 0u)
				return;

			data = new uint8[p_size];
			size = p_size;
		}

		void release()
		{
			delete[]static_cast<uint8 *>(data);
			data = nullptr;
			size = 0u;
		}

		static DataBuffer copy(const DataBuffer &other)
		{
			DataBuffer buffer;
			buffer.alloc(other.size);
			std::memcpy(buffer.data, other.data, other.size);
			return buffer;
		}

		static DataBuffer copy(const void *p_data, uint64 p_size)
		{
			DataBuffer buff;
			buff.alloc(p_size);
			std::memcpy(buff.data, p_data, p_size);
			return buff;
		}

		template<typename Type>
		Type &read(uint64 p_offset = 0u)
		{
			return *static_cast<Type *>(static_cast<uint8 *>(data) + p_offset);
		}

		template<typename Type>
		const Type &read(uint64 p_offset = 0u) const
		{
			return *static_cast<Type *>(static_cast<uint8 *>(data) + p_offset);
		}

		[[nodiscard]] uint8 *readBytes(uint64 p_size, uint64 p_offset) const
		{
			TST_ASSERT(p_offset + p_size <= size);
			auto buffer = new uint8[p_size];
			std::memcpy(buffer, static_cast<uint8 *>(data) + p_offset, p_size);
			return buffer;
		}

		void write(const void *p_data, uint64 p_size, uint64 p_offset = 0u)
		{
			TST_ASSERT(p_offset + p_size <= size);
			std::memcpy(static_cast<uint8 *>(data) + p_offset, p_data, p_size);
		}

		operator bool() const
		{
			return data;
		}

		uint8 &operator[](int32 p_index)
		{
			return static_cast<uint8 *>(data)[p_index];
		}

		uint8 operator[](int32 p_index) const
		{
			return static_cast<uint8 *>(data)[p_index];
		}

		template<typename Type>
		Type *as() const
		{
			return static_cast<Type *>(data);
		}

		[[nodiscard]] uint64 getSize() const { return size; }

		void zeroInit()
		{
			if (data)
				std::memset(data, 0, size);
		}
	};
}
