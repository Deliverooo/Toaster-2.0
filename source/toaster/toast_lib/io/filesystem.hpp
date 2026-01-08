#pragma once

#include <filesystem>
#include <fstream>

#include "system_types.h"
#include "toast_assert.h"

namespace toaster::io::filesystem
{
	using Path = std::filesystem::path;

	Path getWorkingDirectory();
	void setWorkingDirectory(const Path &p_dir);

	void createDirectory(const Path &p_dir);

	bool exists(const Path &p_path);

	std::string readFile(const Path &p_path);
}
