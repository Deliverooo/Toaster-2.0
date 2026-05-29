#pragma once

#include "../system_types.h"
#include "../io/filesystem.hpp"

namespace toaster::os
{
	struct TST_LIB_API FileDialogFilterItem
	{
		const char *name;
		const char *spec;
	};

	TST_LIB_API auto openFileDialog(std::initializer_list<FileDialogFilterItem> p_in_filters = {}) -> io::filesystem::Path;
	TST_LIB_API auto openFolderDialog(const char *p_initial_folder = "") -> io::filesystem::Path;
	TST_LIB_API auto saveFileDialog(std::initializer_list<FileDialogFilterItem> p_in_filters = {}) -> io::filesystem::Path;
}
