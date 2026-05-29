#pragma once

namespace toaster::io
{
	class StreamWriter;
	class StreamReader;

	// An interface representing an object that can be serialized / deserialized
	class TST_LIB_API Serializable
	{
	public:
		virtual ~Serializable() = default;

		// See stream_writer.hpp
		virtual auto serialize(StreamWriter *writer) const -> void = 0;

		// See stream_reader.hpp
		virtual auto deserialize(StreamReader *reader) -> void = 0;
	};
}
