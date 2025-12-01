#pragma once

namespace tst
{
	class Serializable
	{
	public:
		virtual      ~Serializable() = default;
		virtual void serialize(class StreamWriter *writer) const = 0;
		virtual void deserialize(class StreamReader *reader) = 0;
	};
}
