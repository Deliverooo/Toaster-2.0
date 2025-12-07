#pragma once

#include <map>
#include <unordered_map>

#include "serializable.hpp"
#include "buffer.hpp"
#include "string.hpp"

namespace tst
{
	// Essentially a wrapper for std::ostream#
	// The specific implementations would be FileStreamWriter...
	class StreamWriter
	{
	public:
		virtual ~StreamWriter() = default;

		// Example from FileStreamWriter: return m_stream.good()
		// see std::ofstream::good()
		[[nodiscard]] virtual bool isStreamGood() const = 0;

		// Returns the current position in the stream
		virtual uint64 getCurrentStreamPos() = 0;

		// Sets the current position in the stream
		virtual void setCurrentStreamPos(uint64 stream_pos) = 0;

		// Writes data from the buffer into current stream
		virtual bool writeData(const int8 *data, uint64 size) = 0;

		// Writes data from the buffer into current stream
		// If write_size is true, the buffer's size will be written aswell
		void writeBuffer(Buffer buffer, bool write_size = true);

		void writeZero(uint64_t size);
		void writeString(const String &string);

		template<typename Type>
		void writeRaw(const Type &type)
		{
			const bool success = writeData(reinterpret_cast<const int8 *>(&type), sizeof(type));
			TST_ASSERT(success);
		}

		template<typename Type>
		void writeObject(const Type &obj)
		{
			obj.serialize(this);
		}

		template<typename TKey, typename TValue>
		void writeMap(const std::map<TKey, TValue> &map, const bool write_size = true)
		{
			if (write_size)
				writeRaw<uint32>(static_cast<uint32>(map.size()));

			for (const auto &[key, value]: map)
			{
				if constexpr (std::is_trivial<TKey>())
				{
					writeRaw<TKey>(key);
				}
				else
				{
					writeObject<TKey>(key);
				}

				if constexpr (std::is_trivial<TValue>())
				{
					writeRaw<TValue>(value);
				}
				else
				{
					writeObject<TValue>(value);
				}
			}
		}

		template<typename TKey, typename TValue>
		void writeMap(const std::unordered_map<TKey, TValue> &map, bool write_size = true)
		{
			if (write_size)
				writeRaw<uint32>(static_cast<uint32>(map.size()));

			for (const auto &[key, value]: map)
			{
				if constexpr (std::is_trivial<TKey>())
				{
					writeRaw<TKey>(key);
				}
				else
				{
					writeObject<TKey>(key);
				}

				if constexpr (std::is_trivial<TValue>())
				{
					writeRaw<TValue>(value);
				}
				else
				{
					writeObject<TValue>(value);
				}
			}
		}

		template<typename TValue>
		void writeMap(const std::unordered_map<String, TValue> &map, bool write_size = true)
		{
			if (write_size)
				writeRaw<uint32>(static_cast<uint32>(map.size()));

			for (const auto &[key, value]: map)
			{
				writeString(key);

				if constexpr (std::is_trivial<TValue>())
				{
					writeRaw<TValue>(value);
				}
				else
				{
					writeObject<TValue>(value);
				}
			}
		}

		template<typename TValue>
		void writeMap(const std::map<String, TValue> &map, const bool write_size = true)
		{
			if (write_size)
				writeRaw<uint32>(static_cast<uint32>(map.size()));

			for (const auto &[key, value]: map)
			{
				writeString(key);

				if constexpr (std::is_trivial<TValue>())
				{
					writeRaw<TValue>(value);
				}
				else
				{
					writeObject<TValue>(value);
				}
			}
		}

		template<typename Type>
		void writeArray(const std::vector<Type> &array, const bool write_size = true)
		{
			if (write_size)
				writeRaw<uint32>(static_cast<uint32>(array.size()));

			for (const auto &val: array)
			{
				if constexpr (std::is_trivial<Type>())
				{
					writeRaw<Type>(val);
				}
				else
				{
					writeObject<Type>(val);
				}
			}
		}
	};

	template<>
	inline void StreamWriter::writeArray(const std::vector<String> &array, const bool write_size)
	{
		if (write_size)
			writeRaw<uint32>(static_cast<uint32>(array.size()));

		for (const auto &val: array)
		{
			writeString(val);
		}
	}
}
