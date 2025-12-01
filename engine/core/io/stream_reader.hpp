#pragma once

#include <map>
#include <unordered_map>

#include "serializable.hpp"
#include "memory/buffer.hpp"
#include "string/string.hpp"

namespace tst
{
	class StreamReader
	{
	public:
		virtual      ~StreamReader();
		virtual bool isStreamGood() const = 0;

		virtual uint64 getCurrentStreamPos() const = 0;
		virtual void   setCurrentStreamPos() = 0;

		virtual bool readData(uint8 *dst, uint64 size) = 0; // Reads data from the current stream position into the destination buffer

		void readBuffer(Buffer &dst_buffer, uint32 size = 0u);
		void readString(String &dst_string);

		template<typename Type>
		void readRaw(Type &dst_type)
		{
			const bool success = readData(reinterpret_cast<uint8 *>(&dst_type), sizeof(Type));
			TST_ASSERT(success);
		}

		template<typename Type> requires std::is_base_of_v<Serializable, Type>
		void readObject(Type &obj)
		{
			obj.deserialize(this);
		}

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

		template<typename Type>
		void writeArray(std::vector<Type> &array, uint32 size = 0)
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

	template<>
	inline void StreamReader::writeArray(std::vector<String> &array, uint32 size)
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
