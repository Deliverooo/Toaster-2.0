#pragma once

namespace tst
{
	// An interface representing an object that can be serialized / deserialized
	class Serializable
	{
	public:
		virtual ~Serializable() = default;

		// See stream_writer.hpp
		virtual void serialize(class StreamWriter *writer) const = 0;

		// See stream_reader.hpp
		virtual void deserialize(class StreamReader *reader) = 0;
	};
}
