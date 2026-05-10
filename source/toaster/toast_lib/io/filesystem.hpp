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
