#include "filesystem.hpp"

namespace toaster::io::filesystem
{
	static int skipByteOrderMark(std::istream &p_in)
	{
		char test[4] = {0};
		p_in.seekg(0, std::ios::beg);
		p_in.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			p_in.seekg(3, std::ios::beg);
			return 3;
		}
		p_in.seekg(0, std::ios::beg);
		return 0;
	}

	Path getWorkingDirectory()
	{
		return std::filesystem::current_path();
	}

	void setWorkingDirectory(const Path &p_dir)
	{
		std::filesystem::current_path(p_dir);
	}

	void createDirectory(const Path &p_dir)
	{
		std::filesystem::create_directory(p_dir);
	}

	bool exists(const Path &p_path)
	{
		return std::filesystem::exists(p_path);
	}

	std::string readFile(const Path &p_path)
	{
		std::string   result;
		std::ifstream in{p_path, std::ios::in | std::ios::binary};
		if (in)
		{
			in.seekg(0, std::ios::end);
			auto      fileSize     = in.tellg();
			const int skippedChars = skipByteOrderMark(in);

			fileSize -= skippedChars - 1;
			result.resize(fileSize);
			in.read(result.data() + 1, fileSize);

			result[0] = '\t';
		}
		in.close();
		return result;
	}
}
