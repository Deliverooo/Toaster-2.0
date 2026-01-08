#include "file_stream.hpp"

#include <utility>

namespace toaster::io
{
	FileStreamReader::FileStreamReader(filesystem::Path p_path) : m_path(std::move(p_path))
	{
		m_fileStream = std::ifstream(m_path, std::ios::in | std::ios::binary);
	}

	FileStreamReader::~FileStreamReader()
	{
		m_fileStream.close();
	}

	bool FileStreamReader::isGood() const
	{
		return m_fileStream.good();
	}

	uint64 FileStreamReader::getStreamPos() const
	{
		return m_fileStream.tellg();
	}

	void FileStreamReader::setStreamPos(uint64 p_stream_pos)
	{
		m_fileStream.seekg(static_cast<std::streamoff>(p_stream_pos));
	}

	bool FileStreamReader::readData(uint8 *p_dst, uint64 p_size)
	{
		m_fileStream.read(reinterpret_cast<char *>(p_dst), static_cast<std::streamsize>(p_size));
		return true;
	}

	FileStreamWriter::FileStreamWriter(filesystem::Path p_path) : m_path(std::move(p_path))
	{
		m_fileStream = std::ofstream(m_path, std::ios::out | std::ios::binary);
	}

	FileStreamWriter::~FileStreamWriter()
	{
		m_fileStream.close();
	}

	bool FileStreamWriter::isGood() const
	{
		return m_fileStream.good();
	}

	uint64 FileStreamWriter::getStreamPos() const
	{
		return m_fileStream.tellp();
	}

	void FileStreamWriter::setStreamPos(const uint64 p_stream_pos)
	{
		m_fileStream.seekp(static_cast<std::streamoff>(p_stream_pos));
	}

	bool FileStreamWriter::writeData(const uint8 *p_data, const uint64 p_size)
	{
		m_fileStream.write(reinterpret_cast<const char *>(p_data), static_cast<std::streamsize>(p_size));
		return true;
	}
}
