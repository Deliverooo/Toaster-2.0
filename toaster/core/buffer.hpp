#pragma once

#include "core_typedefs.hpp"
#include "error_macros.hpp"
#include "allocator.hpp"

namespace tst
{
	struct Buffer
	{
		void * data;
		uint64 size;

		Buffer() : data(nullptr), size(0)
		{
		}

		explicit Buffer(const void *data, const uint64 size = 0) : data(const_cast<void *>(data)), size(size)
		{
		}

		void alloc(const uint64 size_p)
		{
			delete[]static_cast<uint8 *>(data);
			data = nullptr;

			if (size_p == 0)
			{
				return;
			}

			data = tnew uint8[size_p];
			size = size_p;
		}

		void realloc(uint64_t size)
		{
			release();
			alloc(size);
		}

		void release()
		{
			delete[]static_cast<uint8 *>(data);
			data = nullptr;
			size = 0;
		}

		static Buffer copy(const Buffer &other)
		{
			Buffer buffer;
			buffer.alloc(other.size);
			std::memcpy(buffer.data, other.data, other.size);
			return buffer;
		}

		static Buffer copy(const void *data_p, const uint32 size_p)
		{
			Buffer buff;
			buff.alloc(size_p);
			std::memcpy(buff.data, data_p, size_p);
			return buff;
		}

		template<typename T>
		T &read(const uint64 offset = 0)
		{
			return *reinterpret_cast<T *>(static_cast<uint8 *>(data) + offset);
		}

		template<typename T>
		const T &read(const uint64 offset = 0) const
		{
			return *reinterpret_cast<T *>(static_cast<uint8 *>(data) + offset);
		}

		[[nodiscard]] uint8 *readBytes(const uint64 size_p, const uint64 offset) const
		{
			TST_ASSERT(offset + size_p <= size);
			const auto buffer = tnew uint8[size_p];
			std::memcpy(buffer, static_cast<uint8 *>(data) + offset, size_p);
			return buffer;
		}

		void write(const void *data_p, const uint64 size_p, const uint64 offset = 0)
		{
			TST_ASSERT(offset + size_p <= size);
			std::memcpy(static_cast<uint8 *>(data) + offset, data_p, size_p);
		}

		void write(Buffer buffer, uint64_t offset = 0)
		{
			TST_ASSERT(offset + buffer.size <= size);
			std::memcpy(static_cast<uint8 *>(data) + offset, buffer.data, buffer.size);
		}

		operator bool() const
		{
			return data;
		}

		uint8 &operator[](const int index)
		{
			return static_cast<uint8 *>(data)[index];
		}

		uint8 operator[](const int index) const
		{
			return static_cast<uint8 *>(data)[index];
		}

		template<typename T>
		T *as() const
		{
			return static_cast<T *>(data);
		}

		void zeroInit()
		{
			if (data)
			{
				std::memset(data, 0, size);
			}
		}
	};
}
