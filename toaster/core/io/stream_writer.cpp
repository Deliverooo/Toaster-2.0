#include "stream_writer.hpp"

namespace tst
{
	void StreamWriter::writeBuffer(Buffer buffer, bool write_size)
	{
		if (write_size)
			writeData(reinterpret_cast<int8 *>(&buffer.size), sizeof(uint64));

		writeData(reinterpret_cast<int8 *>(&buffer.data), buffer.size);
	}

	void StreamWriter::writeZero(uint64_t size)
	{
		int8 zero = 0;
		for (uint64_t i = 0; i < size; ++i)
			writeData(reinterpret_cast<const int8 *>(&zero), 1);
	}

	void StreamWriter::writeString(const String &string)
	{
		size_t size = string.size();
		writeData(reinterpret_cast<int8 *>(&size), sizeof(size_t));
		writeData(reinterpret_cast<const int8 *>(string.data()), size);
	}
}
