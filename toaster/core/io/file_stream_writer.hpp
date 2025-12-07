#pragma once

#include "stream_writer.hpp"

#include <filesystem>
#include <fstream>

namespace tst
{
	class FileStreamWriter final : public StreamWriter
	{
	public:
		FileStreamWriter(const std::filesystem::path &file_path) { m_stream = std::ofstream(file_path, std::ifstream::out | std::ifstream::binary); }
		~FileStreamWriter() override { m_stream.close(); }

		uint64 getCurrentStreamPos() override { return m_stream.tellp(); }
		void   setCurrentStreamPos(uint64 stream_pos) override { m_stream.seekp(stream_pos); }
		bool   isStreamGood() const override { return m_stream.good(); }

		bool writeData(const int8 *data, size_t size) override
		{
			m_stream.write(data, size);
			return true;
		}

	private:
		std::filesystem::path m_filePath;
		std::ofstream         m_stream;
	};
}
