#pragma once

#include "string.hpp"

#include <regex>
#include <vector>
#include <fstream>

namespace tst::utils
{
	inline std::vector<String> splitStringAndKeepDelims(String str)
	{
		const static std::regex re(R"((^\W|^\w+)|(\w+)|[:()])", std::regex_constants::optimize);

		std::regex_iterator<String::iterator> rit(str.begin(), str.end(), re);
		std::regex_iterator<String::iterator> rend;
		std::vector<String>                   result;

		while (rit != rend)
		{
			result.emplace_back(rit->str());
			++rit;
		}
		return result;
	}

	inline std::vector<String> splitString(const StringView string, const StringView &delimiters)
	{
		size_t first = 0;

		std::vector<String> result;

		while (first <= string.size())
		{
			const auto second = string.find_first_of(delimiters, first);

			if (first != second)
				result.emplace_back(string.substr(first, second - first));

			if (second == StringView::npos)
				break;

			first = second + 1;
		}

		return result;
	}

	inline std::vector<String> splitString(const StringView string, const char delimiter)
	{
		return splitString(string, String(1, delimiter));
	}

	inline String splitAtUpperCase(StringView string, StringView delimiter, bool ifLowerCaseOnTheRight /*= true*/)
	{
		String str(string);
		for (int i = (int) string.size() - 1; i > 0; --i)
		{
			const auto rightIsLower = [&] { return i < (int) string.size() && std::islower(str[i + 1]); };

			if (std::isupper(str[i]) && (!ifLowerCaseOnTheRight || rightIsLower()))
				str.insert(i, delimiter);
		}

		return str;
	}

	inline String toLower(const StringView &string)
	{
		String result;
		for (const auto &character: string)
		{
			result += std::tolower(character);
		}

		return result;
	}

	inline String toUpper(const StringView &string)
	{
		String result;
		for (const auto &character: string)
		{
			result += std::toupper(character);
		}

		return result;
	}

	inline int skipBOM(std::istream &in)
	{
		char test[4] = {0};
		in.seekg(0, std::ios::beg);
		in.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			in.seekg(3, std::ios::beg);
			return 3;
		}
		in.seekg(0, std::ios::beg);
		return 0;
	}

	inline String readFileAndSkipBOM(const std::filesystem::path &filepath)
	{
		String        result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			auto      fileSize     = in.tellg();
			const int skippedChars = skipBOM(in);

			fileSize -= skippedChars - 1;
			result.resize(fileSize);
			in.read(result.data() + 1, fileSize);
			result[0] = '\t';
		}
		in.close();
		return result;
	}

	inline std::string removeExtension(const std::string &filename)
	{
		return filename.substr(0, filename.find_last_of('.'));
	}
}
