#include "filesystem/filesystem.hpp"

#include <thread>

#include "buffer.hpp"
#include "error_macros.hpp"
#include "log.hpp"
#include "string_utils.hpp"

#ifdef TST_PLATFORM_WINDOWS
#include <Windows.h>
#include <Shlobj.h>
#endif

namespace tst
{
	std::filesystem::path FileSystem::getWorkingDirectory()
	{
		return std::filesystem::current_path();
	}

	void FileSystem::setWorkingDirectory(std::filesystem::path path)
	{
		std::filesystem::current_path(path);
	}

	bool FileSystem::createDirectory(const std::filesystem::path &directory)
	{
		return std::filesystem::create_directories(directory);
	}

	bool FileSystem::createDirectory(const std::string &directory)
	{
		return createDirectory(std::filesystem::path(directory));
	}

	bool FileSystem::move(const std::filesystem::path &oldFilepath, const std::filesystem::path &newFilepath)
	{
		if (FileSystem::exists(newFilepath))
			return false;

		std::filesystem::rename(oldFilepath, newFilepath);
		return true;
	}

	bool FileSystem::copy(const std::filesystem::path &oldFilepath, const std::filesystem::path &newFilepath)
	{
		if (FileSystem::exists(newFilepath))
			return false;

		std::filesystem::copy(oldFilepath, newFilepath);
		return true;
	}

	bool FileSystem::moveFile(const std::filesystem::path &filepath, const std::filesystem::path &dest)
	{
		return move(filepath, dest / filepath.filename());
	}

	bool FileSystem::copyFile(const std::filesystem::path &filepath, const std::filesystem::path &dest)
	{
		return copy(filepath, dest / filepath.filename());
	}

	bool FileSystem::rename(const std::filesystem::path &oldFilepath, const std::filesystem::path &newFilepath)
	{
		return move(oldFilepath, newFilepath);
	}

	bool FileSystem::renameFilename(const std::filesystem::path &oldFilepath, const std::string &newName)
	{
		std::filesystem::path newPath = oldFilepath.parent_path() / std::filesystem::path(newName + oldFilepath.extension().string());
		return rename(oldFilepath, newPath);
	}

	bool FileSystem::exists(const std::filesystem::path &filepath)
	{
		return std::filesystem::exists(filepath);
	}

	bool FileSystem::exists(const std::string &filepath)
	{
		return std::filesystem::exists(std::filesystem::path(filepath));
	}

	bool FileSystem::deleteFile(const std::filesystem::path &filepath)
	{
		if (!FileSystem::exists(filepath))
			return false;

		if (std::filesystem::is_directory(filepath))
			return std::filesystem::remove_all(filepath) > 0;
		return std::filesystem::remove(filepath);
	}

	bool FileSystem::isDirectory(const std::filesystem::path &filepath)
	{
		return std::filesystem::is_directory(filepath);
	}

	EFileStatus FileSystem::tryOpenFileAndWait(const std::filesystem::path &filepath, uint64_t waitms)
	{
		EFileStatus EFileStatus = tryOpenFile(filepath);
		if (EFileStatus == EFileStatus::eLocked)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(operator""ms((unsigned long long) waitms));
			return tryOpenFile(filepath);
		}
		return EFileStatus;
	}

	// returns true <=> fileA was last modified more recently than fileB
	bool FileSystem::isNewer(const std::filesystem::path &fileA, const std::filesystem::path &fileB)
	{
		return std::filesystem::last_write_time(fileA) > std::filesystem::last_write_time(fileB);
	}

	bool FileSystem::showFileInExplorer(const std::filesystem::path &path)
	{
		auto absolutePath = std::filesystem::canonical(path);
		if (!exists(absolutePath))
			return false;

		#ifdef TST_PLATFORM_WINDOWS
		std::string cmd = std::format("explorer.exe /select,\"{0}\"", absolutePath.string());

		#endif
		system(cmd.c_str());
		return true;
	}

	bool FileSystem::openDirectoryInExplorer(const std::filesystem::path &path)
	{
		#ifdef TST_PLATFORM_WINDOWS
		auto absolutePath = std::filesystem::canonical(path);
		if (!exists(absolutePath))
			return false;

		ShellExecute(NULL, reinterpret_cast<LPCSTR>(L"explore"), reinterpret_cast<LPCSTR>(absolutePath.c_str()), NULL, NULL, SW_SHOWNORMAL);
		return true;

		#endif
	}

	bool FileSystem::openExternally(const std::filesystem::path &path)
	{
		auto absolutePath = std::filesystem::canonical(path);
		if (!exists(absolutePath))
			return false;

		#ifdef TST_PLATFORM_WINDOWS
		ShellExecute(NULL, reinterpret_cast<LPCSTR>(L"open"), reinterpret_cast<LPCSTR>(absolutePath.c_str()), NULL, NULL, SW_SHOWNORMAL);
		return true;
		#endif
	}

	std::filesystem::path FileSystem::getUniqueFileName(const std::filesystem::path &filepath)
	{
		if (!FileSystem::exists(filepath))
			return filepath;

		int  counter = 0;
		auto checkID = [&counter, filepath](auto checkID) -> std::filesystem::path
		{
			++counter;
			const std::string counterStr = [&counter]
			{
				if (counter < 10)
					return "0" + std::to_string(counter);
				else
					return std::to_string(counter);
			}(); // Pad with 0 if < 10;

			std::string newFileName = std::format("{} ({})", utils::removeExtension(filepath.filename().string()), counterStr);

			if (filepath.has_extension())
				newFileName = std::format("{}{}", newFileName, filepath.extension().string());

			if (std::filesystem::exists(filepath.parent_path() / newFileName))
				return checkID(checkID);
			else
				return filepath.parent_path() / newFileName;
		};

		return checkID(checkID);
	}

	uint64_t FileSystem::getLastWriteTime(const std::filesystem::path &filepath)
	{
		TST_ASSERT(FileSystem::exists(filepath));

		if (tryOpenFileAndWait(filepath) == EFileStatus::eSuccess)
		{
			std::filesystem::file_time_type lastWriteTime = std::filesystem::last_write_time(filepath);
			return std::chrono::duration_cast<std::chrono::seconds>(lastWriteTime.time_since_epoch()).count();
		}

		TST_ERROR("FileSystem::GetLastWriteTime - could not open file: {}", filepath.string());
		return 0;
	}

	#ifdef TST_PLATFORM_WINDOWS

	static std::filesystem::path s_PersistentStoragePath;

	EFileStatus FileSystem::tryOpenFile(const std::filesystem::path &filepath)
	{
		HANDLE fileHandle = CreateFile(reinterpret_cast<LPCSTR>(filepath.c_str()), GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			DWORD errorCode = GetLastError();
			if (errorCode == ERROR_FILE_NOT_FOUND || errorCode == ERROR_PATH_NOT_FOUND)
				return EFileStatus::eInvalid;
			if (errorCode == ERROR_SHARING_VIOLATION)
				return EFileStatus::eLocked;

			return EFileStatus::eOtherError;
		}

		CloseHandle(fileHandle);
		return EFileStatus::eSuccess;
	}

	bool FileSystem::writeBytes(const std::filesystem::path &filepath, const Buffer &buffer)
	{
		std::ofstream stream(filepath, std::ios::binary | std::ios::trunc);

		if (!stream)
		{
			stream.close();
			return false;
		}

		stream.write((char *) buffer.data, buffer.size);
		stream.close();

		return true;
	}

	Buffer FileSystem::readBytes(const std::filesystem::path &filepath)
	{
		Buffer buffer;

		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		TST_ASSERT(stream);

		auto end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		auto size = end - stream.tellg();
		TST_ASSERT(size != 0);

		buffer.alloc((uint32_t) size);
		stream.read((char *) buffer.data, buffer.size);
		stream.close();

		return buffer;
	}

	std::filesystem::path FileSystem::getPersistentStoragePath()
	{
		if (!s_PersistentStoragePath.empty())
			return s_PersistentStoragePath;

		PWSTR   roamingFilePath;
		HRESULT result = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &roamingFilePath);
		TST_ASSERT(result == S_OK);
		s_PersistentStoragePath = roamingFilePath;
		s_PersistentStoragePath /= "Hazelnut";

		if (!std::filesystem::exists(s_PersistentStoragePath))
			std::filesystem::create_directory(s_PersistentStoragePath);

		return s_PersistentStoragePath;
	}

	bool FileSystem::hasEnvironmentVariable(const std::string &key)
	{
		HKEY    hKey;
		LSTATUS lOpenStatus = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_ALL_ACCESS, &hKey);

		if (lOpenStatus == ERROR_SUCCESS)
		{
			lOpenStatus = RegQueryValueExA(hKey, key.c_str(), 0, NULL, NULL, NULL);
			RegCloseKey(hKey);
		}

		return lOpenStatus == ERROR_SUCCESS;
	}

	bool FileSystem::setEnvironmentVariable(const std::string &key, const std::string &value)
	{
		HKEY    hKey;
		LPCSTR  keyPath = "Environment";
		DWORD   createdNewKey;
		LSTATUS lOpenStatus = RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &createdNewKey);
		if (lOpenStatus == ERROR_SUCCESS)
		{
			LSTATUS lSetStatus = RegSetValueExA(hKey, key.c_str(), 0, REG_SZ, (LPBYTE) value.c_str(), (DWORD) (value.length() + 1));
			RegCloseKey(hKey);

			if (lSetStatus == ERROR_SUCCESS)
			{
				SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_BLOCK, 100, NULL);
				return true;
			}
		}

		return false;
	}

	std::string FileSystem::getEnvironmentVariable(const std::string &key)
	{
		const char *value = getenv(key.c_str());
		if (value)
			return std::string(value);
		else
			return {};
	}

	#endif
}
