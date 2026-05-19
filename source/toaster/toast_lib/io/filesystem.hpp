#pragma once

#include "../string.hpp"
#include "../system_types.h"

#include <filesystem>
#include <vector>

namespace toaster::io::filesystem
{
	using Path = std::filesystem::path;

	auto getWorkingDirectory() -> Path;
	auto setWorkingDirectory(const Path &p_dir) -> void;

	auto createDirectory(const Path &p_dir) -> void;

	auto exists(const Path &p_path) -> bool;

	auto readBinary(const Path &p_path) -> std::vector<uint32>;
	auto readFile(const Path &p_path) -> String;

	auto readFileAndSkipBOM(const Path &p_path) -> String;
	auto writeFile(const Path &p_path, const String &p_data) -> void;
}

#include<fmt/format.h>

template<>
struct fmt::formatter<toaster::io::filesystem::Path>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const toaster::io::filesystem::Path &p_path, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "{}", p_path.string());
	}
};
