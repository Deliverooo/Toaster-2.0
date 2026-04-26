#include "file_dialog.hpp"

#include "../logging.hpp"
#include "../toast_assert.h"

#include <nfd.hpp>

namespace toaster::os
{
	auto openFileDialog(const std::initializer_list<FileDialogFilterItem> p_in_filters) -> io::filesystem::Path
	{
		NFD::UniquePath filePath;

		switch (NFD::OpenDialog(filePath, reinterpret_cast<const nfdfilteritem_t *>(p_in_filters.begin()), p_in_filters.size()))
		{
			case NFD_OKAY:
				return filePath.get();
			case NFD_CANCEL:
				return "";
			case NFD_ERROR:
			{
				auto error = NFD::GetError();
				LOG_FATAL("NFD-Extended threw an error: {}", error);
				TST_ASSERT(false);
				return "";
			}
		}
		TST_ASSERT(false);
		return "";
	}

	auto openFolderDialog(const char *p_initial_folder) -> io::filesystem::Path
	{
		NFD::UniquePath filePath;

		switch (NFD::PickFolder(filePath, p_initial_folder))
		{
			case NFD_OKAY:
				return filePath.get();
			case NFD_CANCEL:
				return "";
			case NFD_ERROR:
			{
				auto error = NFD::GetError();
				LOG_FATAL("NFD-Extended threw an error: {}", error);
				TST_ASSERT(false);
				return "";
			}
		}
		TST_ASSERT(false);
		return "";
	}

	auto saveFileDialog(const std::initializer_list<FileDialogFilterItem> p_in_filters) -> io::filesystem::Path
	{
		NFD::UniquePath filePath;

		switch (NFD::SaveDialog(filePath, reinterpret_cast<const nfdfilteritem_t *>(p_in_filters.begin()), p_in_filters.size()))
		{
			case NFD_OKAY:
				return filePath.get();
			case NFD_CANCEL:
				return "";
			case NFD_ERROR:
			{
				auto error = NFD::GetError();
				LOG_FATAL("NFD-Extended threw an error: {}", error);
				TST_ASSERT(false);
				return "";
			}
		}
		TST_ASSERT(false);
		return "";
	}
}
