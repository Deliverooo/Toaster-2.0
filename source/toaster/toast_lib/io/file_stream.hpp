#pragma once

#include "stream_reader.hpp"
#include "stream_writer.hpp"

#include "filesystem.hpp"

#include <fstream>

namespace toaster::io
{
	class FileStreamReader : public StreamReader
	{
	public:
		FileStreamReader(filesystem::Path p_path);
		~FileStreamReader() override;

		[[nodiscard]] bool isGood() const override;

		[[nodiscard]] uint64 getStreamPos() const override;
		void                 setStreamPos(uint64 p_stream_pos) override;

		bool readData(uint8 *p_dst, uint64 p_size) override;

	private:
		mutable std::ifstream m_fileStream;
		filesystem::Path      m_path;
	};

	class FileStreamWriter : public StreamWriter
	{
	public:
		FileStreamWriter(filesystem::Path p_path);
		~FileStreamWriter() override;

		[[nodiscard]] bool isGood() const override;

		[[nodiscard]] uint64 getStreamPos() const override;
		void                 setStreamPos(uint64 p_stream_pos) override;

		bool writeData(const uint8 *p_data, uint64 p_size) override;

	private:
		mutable std::ofstream m_fileStream;
		filesystem::Path      m_path;
	};
}
