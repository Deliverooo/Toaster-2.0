#pragma once

#include "gpu/shader.hpp"
#include "gpu/vulkan/vulkan_shader_utils.hpp"

#include <filesystem>
#include <format>
#include <sstream>
#include <unordered_set>

#include "core/log.hpp"
#include "core/string_utils.hpp"

enum VkShaderStageFlagBits;

namespace tst
{
	namespace preprocessor_utils
	{
		template<bool RemoveHeaderGuard = false>
		bool containsHeaderGuard(String &header)
		{
			size_t pos = header.find('#');
			while (pos != String::npos)
			{
				const size_t endOfLine = header.find_first_of("\r\n", pos) + 1;
				auto         tokens    = utils::splitStringAndKeepDelims(header.substr(pos, endOfLine - pos));
				auto         it        = tokens.begin();

				if (*(++it) == "pragma")
				{
					if (*(++it) == "once")
					{
						if constexpr (RemoveHeaderGuard)
							header.erase(pos, endOfLine - pos);
						return true;
					}
				}
				pos = header.find('#', pos + 1);
			}
			return false;
		}

		enum class EState : int8
		{
			eSlashOC,
			eStarIC,
			eSingleLineComment,
			eMultiLineComment,
			eNotAComment
		};

		template<typename InputIt, typename OutputIt>
		void copyWithoutComments(InputIt first, InputIt last, OutputIt out)
		{
			EState state = EState::eNotAComment;

			while (first != last)
			{
				switch (state)
				{
					case EState::eSlashOC:
						if (*first == '/')
							state = EState::eSingleLineComment;
						else if (*first == '*')
							state = EState::eMultiLineComment;
						else
						{
							state  = EState::eNotAComment;
							*out++ = '/';
							*out++ = *first;
						}
						break;
					case EState::eStarIC:
						if (*first == '/')
							state = EState::eNotAComment;
						else
							state = EState::eMultiLineComment;
						break;
					case EState::eNotAComment:
						if (*first == '/')
							state = EState::eSlashOC;
						else
							*out++ = *first;
						break;
					case EState::eSingleLineComment:
						if (*first == '\n')
						{
							state  = EState::eNotAComment;
							*out++ = '\n';
						}
						break;
					case EState::eMultiLineComment:
						if (*first == '*')
							state = EState::eStarIC;
						else if (*first == '\n')
							*out++ = '\n';
						break;
				}
				++first;
			}
		}
	}

	struct IncludeData
	{
		std::filesystem::path includedFilePath{};
		uint64                includeDepth{};
		bool                  isRelative{false};
		bool                  isGuarded{false};
		uint32                hashValue{};

		nvrhi::ShaderType includedStage{};

		bool operator==(const IncludeData &other) const noexcept
		{
			return this->includedFilePath == other.includedFilePath && this->hashValue == other.hashValue;
		}
	};

	struct HeaderCache
	{
		String            source;
		uint32            sourceHash;
		nvrhi::ShaderType stages;
		bool              isGuarded;
	};
}

namespace std
{
	template<>
	struct hash<tst::IncludeData>
	{
		size_t operator()(const tst::IncludeData &data) const noexcept
		{
			return std::filesystem::hash_value(data.includedFilePath) ^ data.hashValue;
		}
	};
}

namespace tst
{
	class ShaderPreprocessor
	{
	public:
		template<EShaderLanguage Lang>
		static nvrhi::ShaderType preprocessHeader(String &                               contents, bool &isGuarded, std::unordered_set<String> &specialMacros,
												  const std::unordered_set<IncludeData> &includeData, const std::filesystem::path &fullPath);
		template<EShaderLanguage Lang>
		static std::map<nvrhi::ShaderType, String> preprocessShader(const String &source, std::unordered_set<String> &specialMacros);
	};

	template<EShaderLanguage Lang>
	nvrhi::ShaderType ShaderPreprocessor::preprocessHeader(String &                               contents, bool &isGuarded, std::unordered_set<String> &specialMacros,
														   const std::unordered_set<IncludeData> &includeData, const std::filesystem::path &fullPath)
	{
		std::ostringstream sourceStream;
		preprocessor_utils::copyWithoutComments(contents.begin(), contents.end(), std::ostream_iterator<char>(sourceStream));
		contents = sourceStream.str();

		nvrhi::ShaderType stagesInHeader = nvrhi::ShaderType::None;

		//Removes header guard in GLSL only.
		isGuarded = preprocessor_utils::containsHeaderGuard<Lang == EShaderLanguage::eGLSL>(contents);

		uint32_t stageCount         = 0;
		size_t   startOfShaderStage = contents.find('#', 0);

		while (startOfShaderStage != String::npos)
		{
			const size_t endOfLine = contents.find_first_of("\r\n", startOfShaderStage) + 1;
			// Parse stage. example: #pragma stage:vert
			auto tokens = utils::splitStringAndKeepDelims(contents.substr(startOfShaderStage, endOfLine - startOfShaderStage));

			uint32_t index = 0;
			// Pre-processor directives
			if (tokens[index] == "#")
			{
				++index;
				// Pragmas
				if (tokens[index] == "pragma")
				{
					++index;
					// Stages
					if (tokens[index] == "stage")
					{
						TST_ASSERT(tokens[++index] == ":");

						// Skipped ':'
						const StringView stage(tokens[++index]);
						TST_ASSERT(stage == "vert" || stage == "frag" || stage == "comp");
						nvrhi::ShaderType foundStage = shader_utils::preprocessorStageToShaderStage(stage);

						const bool alreadyIncluded = std::find_if(includeData.begin(), includeData.end(), [fullPath, foundStage](const IncludeData &data)
						{
							return data.includedFilePath == fullPath.string() && !bool((uint16_t) foundStage & (uint16_t) data.includedStage);
						}) != includeData.end();

						if (isGuarded && alreadyIncluded)
							contents.clear();
						else if (!isGuarded && alreadyIncluded)
							TST_WARN("\"{}\" Header does not contain a header guard (#pragma once).", fullPath.string());

						// Add #endif
						if (stageCount == 0)
							contents.replace(startOfShaderStage, endOfLine - startOfShaderStage,
											 std::format("#ifdef {}\r\n", shader_utils::shaderStageToShaderMacro(foundStage)));
						else // Add stage macro instead of stage pragma, both #endif and #ifdef must be in the same line, hence no '\n'
							contents.replace(startOfShaderStage, endOfLine - startOfShaderStage,
											 std::format("#endif\r\n#ifdef {}", shader_utils::shaderStageToShaderMacro(foundStage)));

						*(uint16_t *) &stagesInHeader |= (uint16_t) foundStage;
						stageCount++;
					}
				}
				else if (tokens[index] == "ifdef")
				{
					++index;
					if (tokens[index].rfind("__TST_", 0) == 0) // Hazel special macros start with "__HZ_"
					{
						specialMacros.emplace(tokens[index]);
					}
				}
				else if (tokens[index] == "if" || tokens[index] == "define") // Consider "#if defined()" too?
				{
					++index;
					for (size_t i = index; i < tokens.size(); ++i)
					{
						if (tokens[i].rfind("__TST_", 0) == 0) // Hazel special macros start with "__HZ_"
						{
							specialMacros.emplace(tokens[i]);
						}
					}
				}
			}

			startOfShaderStage = contents.find('#', startOfShaderStage + 1);
		}
		if (stageCount)
			contents.append("\n#endif");
		else
		{
			const bool alreadyIncluded = std::find_if(includeData.begin(), includeData.end(), [fullPath](const IncludeData &data)
			{
				return data.includedFilePath == fullPath;
			}) != includeData.end();
			if (isGuarded && alreadyIncluded)
				contents.clear();
			else if (!isGuarded && alreadyIncluded)
				TST_WARN("\"{}\" Header does not contain a header guard (#pragma once)", fullPath.string());
		}

		return stagesInHeader;
	}

	template<EShaderLanguage Lang>
	std::map<nvrhi::ShaderType, String> ShaderPreprocessor::preprocessShader(const String &source, std::unordered_set<String> &specialMacros)
	{
		std::ostringstream sourceStream;
		preprocessor_utils::copyWithoutComments(source.begin(), source.end(), std::ostream_iterator<char>(sourceStream));
		String newSource = sourceStream.str();

		std::map<nvrhi::ShaderType, String>                shaderSources;
		std::vector<std::pair<nvrhi::ShaderType, size_t> > stagePositions;
		TST_ASSERT(newSource.size());

		size_t startOfStage = 0;
		size_t pos          = newSource.find('#');

		//Check first #version
		if constexpr (Lang == EShaderLanguage::eGLSL)
		{
			const size_t              endOfLine = newSource.find_first_of("\r\n", pos) + 1;
			const std::vector<String> tokens    = utils::splitStringAndKeepDelims(newSource.substr(pos, endOfLine - pos));
			TST_ASSERT(tokens.size() >= 3 && tokens[1] == "version");
			pos = newSource.find('#', pos + 1);
		}

		while (pos != String::npos)
		{
			const size_t        endOfLine = newSource.find_first_of("\r\n", pos) + 1;
			std::vector<String> tokens    = utils::splitStringAndKeepDelims(newSource.substr(pos, endOfLine - pos));

			size_t index = 1; // Skip #

			if (tokens[index] == "pragma") // Parse stage. example: #pragma stage : vert
			{
				++index;
				if (tokens[index] == "stage")
				{
					++index;
					// Jump over ':'
					TST_ASSERT(tokens[index] == ":");
					++index;

					const StringView stage = tokens[index];
					TST_ASSERT(stage == "vert" || stage == "frag" || stage == "comp");
					auto shaderStage = shader_utils::preprocessorStageToShaderStage(stage);

					if constexpr (Lang == EShaderLanguage::eHLSL)
					{
						startOfStage = pos;
					}
					stagePositions.emplace_back(shaderStage, startOfStage);
				}
			}
			else if (tokens[index] == "ifdef")
			{
				++index;
				if (tokens[index].rfind("__TST_", 0) == 0) // Hazel special macros start with "__HZ_"
				{
					specialMacros.emplace(tokens[index]);
				}
			}
			else if (tokens[index] == "if" || tokens[index] == "define")
			{
				++index;
				for (size_t i = index; i < tokens.size(); ++i)
				{
					if (tokens[i].rfind("__TST_", 0) == 0) // Hazel special macros start with "__HZ_"
					{
						specialMacros.emplace(tokens[i]);
					}
				}
			}
			else if constexpr (Lang == EShaderLanguage::eGLSL)
			{
				if (tokens[index] == "version")
				{
					++index;
					startOfStage = pos;
				}
			}

			pos = newSource.find('#', pos + 1);
		}

		TST_ASSERT(stagePositions.size());
		auto &[firstStage, firstStagePos] = stagePositions[0];
		if (stagePositions.size() > 1)
		{
			//Get first stage
			const String firstStageStr = newSource.substr(0, stagePositions[1].second);
			size_t       lineCount     = std::count(firstStageStr.begin(), firstStageStr.end(), '\n') + 1;
			shaderSources[firstStage]  = firstStageStr;

			//Get stages in the middle
			for (size_t i = 1; i < stagePositions.size() - 1; ++i)
			{
				auto &       [stage, stagePos] = stagePositions[i];
				String       stageStr          = newSource.substr(stagePos, stagePositions[i + 1].second - stagePos);
				const size_t secondLinePos     = stageStr.find_first_of('\n', 1) + 1;
				stageStr.insert(secondLinePos, std::format("#line {}\n", lineCount));
				shaderSources[stage] = stageStr;
				lineCount += std::count(stageStr.begin(), stageStr.end(), '\n') + 1;
			}

			//Get last stage
			auto &       [stage, stagePos] = stagePositions[stagePositions.size() - 1];
			String       lastStageStr      = newSource.substr(stagePos);
			const size_t secondLinePos     = lastStageStr.find_first_of('\n', 1) + 1;
			lastStageStr.insert(secondLinePos, std::format("#line {}\n", lineCount + 1));
			shaderSources[stage] = lastStageStr;
		}
		else
		{
			shaderSources[firstStage] = newSource;
		}

		return shaderSources;
	}
}
