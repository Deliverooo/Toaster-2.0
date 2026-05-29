#include "toast_lib/io/file_stream.hpp"

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

	auto FileStreamReader::isGood() const -> bool
	{
		return m_fileStream.good();
	}

	auto FileStreamReader::getStreamPos() const -> uint64
	{
		return m_fileStream.tellg();
	}

	auto FileStreamReader::setStreamPos(uint64 p_stream_pos) -> void
	{
		m_fileStream.seekg(static_cast<std::streamoff>(p_stream_pos));
	}

	auto FileStreamReader::readData(uint8 *p_dst, uint64 p_size) -> bool
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

	auto FileStreamWriter::isGood() const -> bool
	{
		return m_fileStream.good();
	}

	auto FileStreamWriter::getStreamPos() const -> uint64
	{
		return m_fileStream.tellp();
	}

	auto FileStreamWriter::setStreamPos(const uint64 p_stream_pos) -> void
	{
		m_fileStream.seekp(static_cast<std::streamoff>(p_stream_pos));
	}

	auto FileStreamWriter::writeData(const uint8 *p_data, const uint64 p_size) -> bool
	{
		m_fileStream.write(reinterpret_cast<const char *>(p_data), static_cast<std::streamsize>(p_size));
		return true;
	}
}
