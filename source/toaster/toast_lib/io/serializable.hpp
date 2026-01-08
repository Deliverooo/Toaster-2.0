#pragma once

namespace toaster::io
{
	class StreamWriter;
	class StreamReader;

	// An interface representing an object that can be serialized / deserialized
	class Serializable
	{
	public:
		virtual ~Serializable() = default;

		// See stream_writer.hpp
		virtual void serialize(StreamWriter *writer) const = 0;

		// See stream_reader.hpp
		virtual void deserialize(StreamReader *reader) = 0;
	};
}
