#include "file_dialog.hpp"

#include "toaster/toast_lib/toast_assert.h"
#include <nfd.hpp>

#include "toaster/toast_lib/logging.hpp"

namespace toaster::os
{
	io::filesystem::Path openFileDialog(const std::initializer_list<FileDialogFilterItem> p_in_filters)
	{
		NFD::UniquePath filePath;
		nfdresult_t     result = NFD::OpenDialog(filePath, (const nfdfilteritem_t *) p_in_filters.begin(), p_in_filters.size());

		switch (result)
		{
			case NFD_OKAY:
				return filePath.get();
			case NFD_CANCEL:
				return "";
			case NFD_ERROR:
			{
				LOG_FATAL("NFD-Extended threw an error: {}", NFD::GetError());
				TST_ASSERT(false);
				return "";
			}
		}
	}

	io::filesystem::Path openFolderDialog(const char *p_initial_folder)
	{
		NFD::UniquePath filePath;
		nfdresult_t     result = NFD::PickFolder(filePath, p_initial_folder);

		switch (result)
		{
			case NFD_OKAY:
				return filePath.get();
			case NFD_CANCEL:
				return "";
			case NFD_ERROR:
			{
				LOG_FATAL("NFD-Extended threw an error: {}", NFD::GetError());
				TST_ASSERT(false);
				return "";
			}
		}
	}

	io::filesystem::Path saveFileDialog(const std::initializer_list<FileDialogFilterItem> p_in_filters)
	{
		NFD::UniquePath filePath;
		nfdresult_t     result = NFD::SaveDialog(filePath, (const nfdfilteritem_t *) p_in_filters.begin(), p_in_filters.size());

		switch (result)
		{
			case NFD_OKAY:
				return filePath.get();
			case NFD_CANCEL:
				return "";
			case NFD_ERROR:
			{
				LOG_FATAL("NFD-Extended threw an error: {}", NFD::GetError());
				TST_ASSERT(false);
				return "";
			}
		}
	}
}
