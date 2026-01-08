#pragma once

#include <string>

#include "system_types.h"
#include "toast_assert.h"

#include "serializable.hpp"

namespace toaster::io
{
	// Essentially a wrapper for std::ostream
	// The specific implementations would be FileStreamWriter...
	class StreamWriter
	{
	public:
		virtual ~StreamWriter() = default;

		// Example from FileStreamWriter: return m_stream.is_good()
		// see std::ofstream::is_good()
		[[nodiscard]] virtual bool isGood() const = 0;
		// Returns the current position in the stream
		[[nodiscard]] virtual uint64 getStreamPos() const = 0;
		// Sets the current position in the stream
		virtual void setStreamPos(uint64 p_stream_pos) = 0;

		// Reads data from the current stream position into the destination buffer
		virtual bool writeData(const uint8 *p_data, uint64 p_size) = 0;

		// writes to the current stream into the destination type by the size of that type
		template<typename Type> requires std::is_trivial_v<Type>
		void writeRaw(const Type &p_type)
		{
			const bool success = writeData(reinterpret_cast<int8 *>(&p_type), sizeof(Type));
			TST_ASSERT_MSG(success, "Failed to write type");
		}

		// writes to the current stream into the destination object
		// This requires that the object derives from the Serializable interface and implements the serialize
		// and deserialize methods
		template<typename TObj> requires std::is_object_v<TObj> && std::derived_from<TObj, Serializable>
		void writeRaw(TObj &p_obj)
		{
			p_obj.serialize(this);
		}

		void writeString(const std::string &p_str)
		{
			// For strings, the size is written before the char buffer so we know how far into the data to read
			uint64 size = p_str.size();
			writeData(reinterpret_cast<uint8 *>(&size), sizeof(uint64));
			writeData(reinterpret_cast<const uint8 *>(p_str.data()), sizeof(char) * size);
		}

		operator bool() const noexcept { return isGood(); }
	};
}
