#pragma once

#include <filesystem>
#include "buffer.hpp"

namespace tst
{
	enum class EFileStatus
	{
		eSuccess = 0,
		eInvalid,
		eLocked,
		eOtherError
	};

	class FileSystem
	{
	public:
		static std::filesystem::path getWorkingDirectory();
		static void                  setWorkingDirectory(std::filesystem::path path);
		static bool                  createDirectory(const std::filesystem::path &directory);
		static bool                  createDirectory(const std::string &directory);
		static bool                  exists(const std::filesystem::path &filepath);
		static bool                  exists(const std::string &filepath);
		static bool                  deleteFile(const std::filesystem::path &filepath);
		static bool                  moveFile(const std::filesystem::path &filepath, const std::filesystem::path &dest);
		static bool                  copyFile(const std::filesystem::path &filepath, const std::filesystem::path &dest);
		static bool                  isDirectory(const std::filesystem::path &filepath);

		static EFileStatus tryOpenFile(const std::filesystem::path &filepath);

		// If file is locked, wait specified duration (ms) and try again once
		static EFileStatus tryOpenFileAndWait(const std::filesystem::path &filepath, uint64_t waitms = 100);

		static bool isNewer(const std::filesystem::path &fileA, const std::filesystem::path &fileB);

		static bool move(const std::filesystem::path &oldFilepath, const std::filesystem::path &newFilepath);
		static bool copy(const std::filesystem::path &oldFilepath, const std::filesystem::path &newFilepath);
		static bool rename(const std::filesystem::path &oldFilepath, const std::filesystem::path &newFilepath);
		static bool renameFilename(const std::filesystem::path &oldFilepath, const std::string &newName);

		static bool showFileInExplorer(const std::filesystem::path &path);
		static bool openDirectoryInExplorer(const std::filesystem::path &path);
		static bool openExternally(const std::filesystem::path &path);

		static bool   writeBytes(const std::filesystem::path &filepath, const Buffer &buffer);
		static Buffer readBytes(const std::filesystem::path &filepath);

		static std::filesystem::path getUniqueFileName(const std::filesystem::path &filepath);
		static uint64_t              getLastWriteTime(const std::filesystem::path &filepath);

	public:
		struct FileDialogFilterItem
		{
			const char *Name;
			const char *Spec;
		};

		static std::filesystem::path getPersistentStoragePath();

	public:
		static bool        hasEnvironmentVariable(const std::string &key);
		static bool        setEnvironmentVariable(const std::string &key, const std::string &value);
		static std::string getEnvironmentVariable(const std::string &key);
	};
}
