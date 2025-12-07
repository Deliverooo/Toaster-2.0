#pragma once
#include "core/ref_ptr.hpp"
#include "gpu/vulkan/vulkan_shader.hpp"

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include "shader_preprocessing/shader_preprocessor.hpp"

struct IDxcCompiler3;
struct IDxcUtils;

namespace tst
{
	struct DxcInstances
	{
		inline static IDxcCompiler3 *Compiler = nullptr;
		inline static IDxcUtils *    Utils    = nullptr;
	};

	struct StageData
	{
		std::unordered_set<IncludeData> Headers;
		uint32_t                        HashValue = 0;
		bool                            operator==(const StageData &other) const noexcept { return this->Headers == other.Headers && this->HashValue == other.HashValue; }
		bool                            operator!=(const StageData &other) const noexcept { return !(*this == other); }
	};

	class VulkanShaderCompiler : public RefCounted
	{
	public:
		VulkanShaderCompiler(const std::filesystem::path &shaderSourcePath, bool disableOptimization = false);

		bool reload(bool forceCompile = false);

		const std::map<nvrhi::ShaderType, std::vector<uint32_t> > &getSPIRVData() const { return m_SPIRVData; }
		const std::unordered_set<std::string> &                    getAcknowledgedMacros() const { return m_AcknowledgedMacros; }

		static void clearUniformBuffers();

		static RefPtr<VulkanShader> compile(const std::filesystem::path &shaderSourcePath, bool forceCompile = false, bool disableOptimization = false);
		static bool                 recompile(RefPtr<VulkanShader> shader);

	private:
		std::map<nvrhi::ShaderType, std::string> preProcess(const std::string &source);
		std::map<nvrhi::ShaderType, std::string> preProcessGLSL(const std::string &source);
		std::map<nvrhi::ShaderType, std::string> preProcessHLSL(const std::string &source);

		struct CompilationOptions
		{
			bool GenerateDebugInfo = false;
			bool Optimize          = true;
		};

		std::string compile(std::vector<uint32_t> &outputBinary, const nvrhi::ShaderType stage, CompilationOptions options) const;
		bool        compileOrGetVulkanBinaries(std::map<nvrhi::ShaderType, std::vector<uint32_t> > &outputDebugBinary,
											   std::map<nvrhi::ShaderType, std::vector<uint32_t> > &outputBinary, const nvrhi::ShaderType changedStages,
											   const bool                                           forceCompile);
		bool compileOrGetVulkanBinary(nvrhi::ShaderType stage, std::vector<uint32_t> &outputBinary, bool debug, nvrhi::ShaderType changedStages, bool forceCompile);

		void clearReflectionData();

		void tryGetVulkanCachedBinary(const std::filesystem::path &cacheDirectory, const std::string &extension, std::vector<uint32_t> &outputBinary) const;
		bool tryReadCachedReflectionData();
		void serializeReflectionData();
		void serializeReflectionData(StreamWriter *serializer);

		void reflectAllShaderStages(const std::map<nvrhi::ShaderType, std::vector<uint32_t> > &shaderData);
		void reflect(nvrhi::ShaderType shaderStage, const std::vector<uint32_t> &shaderData);

	private:
		std::filesystem::path m_shaderSourcePath;
		bool                  m_disableOptimization = false;

		std::map<nvrhi::ShaderType, std::string>            m_shaderSource;
		std::map<nvrhi::ShaderType, std::vector<uint32_t> > m_SPIRVDebugData, m_SPIRVData;

		// Reflection info
		ShaderReflectionData m_ReflectionData;

		// Names of macros that are parsed from shader.
		// These are used to reliably get informattion about what shaders need what macros
		std::unordered_set<std::string> m_AcknowledgedMacros;
		EShaderLanguage                 m_Language;

		std::map<nvrhi::ShaderType, StageData> m_StagesMetadata;

		friend class VulkanShader;
		friend class VulkanShaderCache;
		friend class ShaderPack;
	};
}
