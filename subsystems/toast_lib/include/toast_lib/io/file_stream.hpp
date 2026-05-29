#pragma once

#include "stream_reader.hpp"
#include "stream_writer.hpp"

#include "filesystem.hpp"

#include <fstream>

namespace toaster::io
{
	class TST_LIB_API FileStreamReader final : public StreamReader
	{
	public:
		explicit FileStreamReader(filesystem::Path p_path);
		virtual  ~FileStreamReader() override;

		[[nodiscard]] virtual auto isGood() const -> bool override;

		[[nodiscard]] virtual auto getStreamPos() const -> uint64 override;
		virtual auto               setStreamPos(uint64 p_stream_pos) -> void override;

		virtual auto readData(uint8 *p_dst, uint64 p_size) -> bool override;

	private:
		mutable std::ifstream m_fileStream;
		filesystem::Path      m_path;
	};

	class TST_LIB_API FileStreamWriter final : public StreamWriter
	{
	public:
		explicit FileStreamWriter(filesystem::Path p_path);
		virtual  ~FileStreamWriter() override;

		[[nodiscard]] virtual auto isGood() const -> bool override;

		[[nodiscard]] virtual auto getStreamPos() const -> uint64 override;
		virtual auto               setStreamPos(uint64 p_stream_pos) -> void override;

		virtual auto writeData(const uint8 *p_data, uint64 p_size) -> bool override;

	private:
		mutable std::ofstream m_fileStream;
		filesystem::Path      m_path;
	};
}
