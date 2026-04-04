#pragma once

#include "../system_types.h"

#include <filesystem>
#include "../string.hpp"

namespace toaster::io::filesystem
{
	using Path = std::filesystem::path;

	Path getWorkingDirectory();
	void setWorkingDirectory(const Path &p_dir);

	void createDirectory(const Path &p_dir);

	bool exists(const Path &p_path);

	std::vector<uint32> readBinary(const Path &p_path);
	String              readFile(const Path &p_path);

	String readFileAndSkipBOM(const Path &p_path);
	void   writeFile(const Path &p_path, const String &p_data);
}
