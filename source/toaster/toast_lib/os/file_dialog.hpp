#pragma once

#include "../system_types.h"
#include "../io/filesystem.hpp"

namespace toaster::os
{
	struct FileDialogFilterItem
	{
		const char *name;
		const char *spec;
	};

	auto openFileDialog(std::initializer_list<FileDialogFilterItem> p_in_filters = {}) -> io::filesystem::Path;
	auto openFolderDialog(const char *p_initial_folder = "") -> io::filesystem::Path;
	auto saveFileDialog(std::initializer_list<FileDialogFilterItem> p_in_filters = {}) -> io::filesystem::Path;
}
