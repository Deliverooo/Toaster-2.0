#pragma once

#include <map>
#include <unordered_map>

#include "serializable.hpp"
#include "buffer.hpp"
#include "string.hpp"

namespace tst
{
	// Essentially a wrapper for std::istream
	// The specific implementations would be FileStreamReader...
	class StreamReader
	{
	public:
		virtual ~StreamReader() = default;

		// Example from FileStreamReader: return m_stream.good()
		// see std::ifstream::good()
		virtual bool isStreamGood() const = 0;

		// Returns the current position in the stream
		virtual uint64 getCurrentStreamPos() = 0;

		// Sets the current position in the stream
		virtual void setCurrentStreamPos(uint64 stream_pos) = 0;

		// Reads data from the current stream position into the destination buffer
		virtual bool readData(int8 *dst, uint64 size) = 0;

		// Reads from the current stream into the destination buffer up to thr specified size
		void readBuffer(Buffer &dst_buffer, uint32 size = 0u);

		// Reads from the current stream into the destination string
		void readString(String &dst_string);

		// reads from the current stream into the destination type by the size of that type
		template<typename Type>
		void readRaw(Type &dst_type)
		{
			const bool success = readData(reinterpret_cast<int8 *>(&dst_type), sizeof(Type));
			TST_ASSERT(success);
		}

		// read the current stream into the destination object
		// This requires that the object derives from the Serializable interface and implements the serialize
		// and deserialize methods
		template<typename Type> requires std::is_base_of_v<Serializable, Type>
		void readObject(Type &obj)
		{
			obj.deserialize(this);
		}

		// reads from the current stream into the destination map
		template<typename TKey, typename TValue>
		void readMap(std::map<TKey, TValue> &map, uint32 size = 0)
		{
			if (size == 0)
				readRaw<uint32>(size);

			for (uint32 i = 0; i < size; i++)
			{
				TKey key;
				if constexpr (std::is_trivial_v<TKey>)
				{
					readRaw<TKey>(key);
				}
				else
				{
					readObject<TKey>(key);
				}

				if constexpr (std::is_trivial_v<TValue>)
				{
					readRaw<TValue>(map[key]);
				}
				else
				{
					readObject<TValue>(map[key]);
				}
			}
		}

		// reads from the current stream into the destination unordered_map
		template<typename TKey, typename TValue>
		void readMap(std::unordered_map<TKey, TValue> &map, uint32 size = 0)
		{
			if (size == 0)
				readRaw<uint32>(size);

			for (uint32 i = 0; i < size; i++)
			{
				TKey key;
				if constexpr (std::is_trivial_v<TKey>)
				{
					readRaw<TKey>(key);
				}
				else
				{
					readObject<TKey>(key);
				}

				if constexpr (std::is_trivial_v<TValue>)
				{
					readRaw<TValue>(map[key]);
				}
				else
				{
					readObject<TValue>(map[key]);
				}
			}
		}

		// reads from the current stream into the destination map
		// as String is a known object that we can deserialize,
		// we can partially specialize the template to explicitly read the key back as a String
		template<typename TValue>
		void readMap(std::map<String, TValue> &map, uint32 size = 0)
		{
			if (size == 0)
				readRaw<uint32>(size);

			for (uint32 i = 0; i < size; i++)
			{
				String key;
				readString(key);

				if constexpr (std::is_trivial_v<TValue>)
				{
					readRaw<TValue>(map[key]);
				}
				else
				{
					readObject<TValue>(map[key]);
				}
			}
		}

		// reads from the current stream into the destination map
		// as String is a known object that we can deserialize,
		// we can partially specialize the template to explicitly read the key back as a String
		template<typename TValue>
		void readMap(std::unordered_map<String, TValue> &map, uint32 size = 0)
		{
			if (size == 0)
				readRaw<uint32>(size);

			for (uint32 i = 0; i < size; i++)
			{
				String key;
				readString(key);

				if constexpr (std::is_trivial_v<TValue>)
				{
					readRaw<TValue>(map[key]);
				}
				else
				{
					readObject<TValue>(map[key]);
				}
			}
		}

		// reads from the current stream into the destination vector
		template<typename Type>
		void readArray(std::vector<Type> &array, uint32 size = 0)
		{
			if (size == 0)
				readRaw<uint32>(size);

			array.resize(size);

			for (uint32 i = 0; i < size; i++)
			{
				if constexpr (std::is_trivial_v<Type>)
				{
					readRaw<Type>(array[i]);
				}
				else
				{
					readObject<Type>(array[i]);
				}
			}
		}
	};

	// reads from the current stream into the destination vector
	// as String is a known object that we can deserialize,
	// we can partially specialize the template to explicitly read the indices back as Strings
	template<>
	inline void StreamReader::readArray(std::vector<String> &array, uint32 size)
	{
		if (size == 0)
			readRaw<uint32>(size);

		array.resize(size);

		for (uint32 i = 0; i < size; i++)
		{
			readString(array[i]);
		}
	}
}
