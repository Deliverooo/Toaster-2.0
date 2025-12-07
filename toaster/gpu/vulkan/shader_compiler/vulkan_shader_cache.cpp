#include "vulkan_shader_cache.hpp"
#include "core/serialization_macros.hpp"

#include <yaml-cpp/yaml.h>

#include <nvrhi/utils.h>

namespace tst
{
	static const char *s_shaderRegistryPath = "res/_cache/shader/shader_registry.tscache";

	nvrhi::ShaderType VulkanShaderCache::hasChanged(RefPtr<VulkanShaderCompiler> shader)
	{
		std::map<std::string, std::map<nvrhi::ShaderType, StageData> > shaderCache;

		deserialize(shaderCache);

		nvrhi::ShaderType changedStages   = nvrhi::ShaderType::None;
		const bool        shaderNotCached = shaderCache.find(shader->m_shaderSourcePath.string()) == shaderCache.end();

		for (const auto &[stage, stageSource]: shader->m_shaderSource)
		{
			// Keep in mind that we're using the [] operator.
			// Which means that we add the stage if it's not already there.
			if (shaderNotCached || shader->m_StagesMetadata.at(stage) != shaderCache[shader->m_shaderSourcePath.string()][stage])
			{
				shaderCache[shader->m_shaderSourcePath.string()][stage] = shader->m_StagesMetadata.at(stage);
				*(uint16_t *) &changedStages |= (uint16_t) stage;
			}
		}

		// Update cache in case we added a stage but didn't remove the deleted(in file) stages
		shaderCache.at(shader->m_shaderSourcePath.string()) = shader->m_StagesMetadata;

		if (changedStages != nvrhi::ShaderType::None)
			serialize(shaderCache);

		return changedStages;
	}

	void VulkanShaderCache::serialize(const std::map<std::string, std::map<nvrhi::ShaderType, StageData> > &shaderCache)
	{
		YAML::Emitter out;

		out << YAML::BeginMap << YAML::Key << "shaderRegistry" << YAML::BeginSeq; // ShaderRegistry_

		for (auto &[filepath, shader]: shaderCache)
		{
			out << YAML::BeginMap; // Shader_

			out << YAML::Key << "shaderPath" << YAML::Value << filepath;

			out << YAML::Key << "stages" << YAML::BeginSeq; // Stages_

			for (auto &[stage, stageData]: shader)
			{
				out << YAML::BeginMap; // Stage_

				out << YAML::Key << "stage" << YAML::Value << nvrhi::utils::ShaderStageToString(stage);
				out << YAML::Key << "stageHash" << YAML::Value << stageData.HashValue;

				out << YAML::Key << "headers" << YAML::BeginSeq; // Headers_
				for (auto &header: stageData.Headers)
				{
					out << YAML::BeginMap;

					TST_SERIALIZE_PROPERTY(HeaderPath, header.includedFilePath.string(), out);
					TST_SERIALIZE_PROPERTY(IncludeDepth, header.includeDepth, out);
					TST_SERIALIZE_PROPERTY(IsRelative, header.isRelative, out);
					TST_SERIALIZE_PROPERTY(IsGaurded, header.isGuarded, out);
					TST_SERIALIZE_PROPERTY(HashValue, header.hashValue, out);

					out << YAML::EndMap;
				}
				out << YAML::EndSeq; // Headers_

				out << YAML::EndMap; // Stage_
			}
			out << YAML::EndSeq; // Stages_
			out << YAML::EndMap; // Shader_
		}
		out << YAML::EndSeq; // ShaderRegistry_
		out << YAML::EndMap; // File_

		std::ofstream fout(s_shaderRegistryPath);
		fout << out.c_str();
	}

	void VulkanShaderCache::deserialize(std::map<std::string, std::map<nvrhi::ShaderType, StageData> > &shaderCache)
	{
		// Read registry
		std::ifstream stream(s_shaderRegistryPath);
		if (!stream.good())
			return;

		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data    = YAML::Load(strStream.str());
		auto       handles = data["shaderRegistry"];
		if (handles.IsNull())
		{
			TST_ERROR("[ShaderCache] Shader Registry is invalid.");
			return;
		}

		// Old format
		if (handles.IsMap())
		{
			TST_ERROR("[ShaderCache] Old Shader Registry format.");
			return;
		}

		for (auto shader: handles)
		{
			std::string path;
			TST_DESERIALIZE_PROPERTY(shaderPath, path, shader, std::string());
			for (auto stage: shader["stages"]) //Stages
			{
				std::string stageType;
				uint32_t    stageHash;
				TST_DESERIALIZE_PROPERTY(stage, stageType, stage, std::string());
				TST_DESERIALIZE_PROPERTY(stageHash, stageHash, stage, 0u);

				auto &stageCache     = shaderCache[path][nvrhi::utils::ShaderStageFromString(stageType.c_str())];
				stageCache.HashValue = stageHash;

				for (auto header: stage["headers"])
				{
					std::string headerPath;
					uint32_t    includeDepth;
					bool        isRelative;
					bool        isGuarded;
					uint32_t    hashValue;
					TST_DESERIALIZE_PROPERTY(headerPath, headerPath, header, std::string());
					TST_DESERIALIZE_PROPERTY(includeDepth, includeDepth, header, 0u);
					TST_DESERIALIZE_PROPERTY(isRelative, isRelative, header, false);
					TST_DESERIALIZE_PROPERTY(isGaurded, isGuarded, header, false);
					TST_DESERIALIZE_PROPERTY(hashValue, hashValue, header, 0u);

					stageCache.Headers.emplace(IncludeData{headerPath, includeDepth, isRelative, isGuarded, hashValue});
				}
			}
		}
	}
}
