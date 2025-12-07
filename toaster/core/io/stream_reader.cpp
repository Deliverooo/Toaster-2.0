#include "stream_reader.hpp"

namespace tst
{
	void StreamReader::readBuffer(Buffer &dst_buffer, uint32 size)
	{
		dst_buffer.size = size;
		if (size == 0)
			readData(reinterpret_cast<int8 *>(&dst_buffer.size), sizeof(uint64));

		dst_buffer.alloc(dst_buffer.size);
		readData(static_cast<int8 *>(dst_buffer.data), dst_buffer.size);
	}

	void StreamReader::readString(String &dst_string)
	{
		size_t size;
		readData(reinterpret_cast<int8 *>(&size), sizeof(size_t));

		dst_string.resize(size);
		readData((int8 *) dst_string.data(), size);
	}
}
