#pragma once

#include "../string.hpp"
#include "../system_types.h"
#include "../toast_assert.h"

#include "serializable.hpp"

namespace toaster::io
{
	// Essentially a wrapper for std::istream
	// The specific implementations would be FileStreamReader...
	class StreamReader
	{
	public:
		virtual ~StreamReader() = default;

		// Example from FileStreamReader: return m_stream.is_good()
		// see std::ifstream::is_good()
		[[nodiscard]] virtual auto isGood() const -> bool = 0;

		// Returns the current position in the stream
		[[nodiscard]] virtual auto getStreamPos() const -> uint64 = 0;
		// Sets the current position in the stream
		virtual auto setStreamPos(uint64 p_stream_pos) -> void = 0;

		// Reads data from the current stream position into the destination buffer
		virtual auto readData(uint8 *p_dst, uint64 p_size) -> bool = 0;

		// reads from the current stream into the destination type by the size of that type
		template<typename Type> requires std::is_trivial_v<Type>
		auto read(Type &p_out_type) -> void
		{
			const bool success = readData(reinterpret_cast<uint8 *>(&p_out_type), sizeof(Type));
			TST_ASSERT_MSG(success, "Failed to read type");
		}

		// read the current stream into the destination object
		// This requires that the object derives from the Serializable interface and implements the serialize
		// and deserialize methods
		template<typename TObj> requires std::is_object_v<TObj> && std::derived_from<TObj, Serializable>
		auto readRaw(TObj &p_out_obj) -> void
		{
			p_out_obj.deserialize(this);
		}

		auto readString(String &p_out_str) -> void
		{
			// For strings, the size is written before the char buffer so we know how far into the data to read
			uint64 size;
			readData(reinterpret_cast<uint8 *>(&size), sizeof(uint64));

			p_out_str.resize(size);
			readData(reinterpret_cast<uint8 *>(p_out_str.data()), sizeof(char) * size);
		}

		operator bool() const noexcept { return isGood(); }
	};
}
