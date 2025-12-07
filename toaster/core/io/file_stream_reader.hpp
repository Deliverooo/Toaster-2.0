#pragma once

#include "stream_reader.hpp"

#include <filesystem>
#include <fstream>

namespace tst
{
	class FileStreamReader final : public StreamReader
	{
	public:
		FileStreamReader(const std::filesystem::path &file_path) { m_stream = std::ifstream(file_path, std::ifstream::in | std::ifstream::binary); }
		~FileStreamReader() override { m_stream.close(); }

		uint64 getCurrentStreamPos() override { return m_stream.tellg(); }
		void   setCurrentStreamPos(uint64 stream_pos) override { m_stream.seekg(stream_pos); }
		bool   isStreamGood() const override { return m_stream.good(); }

		bool readData(int8 *dst, size_t size) override
		{
			m_stream.read(dst, size);
			return true;
		}

	private:
		std::filesystem::path m_filePath;
		std::ifstream         m_stream;
	};
}
