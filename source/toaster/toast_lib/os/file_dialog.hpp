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

	io::filesystem::Path openFileDialog(std::initializer_list<FileDialogFilterItem> p_in_filters = {});
	io::filesystem::Path openFolderDialog(const char *p_initial_folder = "");
	io::filesystem::Path saveFileDialog(std::initializer_list<FileDialogFilterItem> p_in_filters = {});
}
