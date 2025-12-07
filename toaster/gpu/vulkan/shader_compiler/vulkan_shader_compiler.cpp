#include "vulkan_shader_compiler.hpp"
#include "shader_preprocessing/shader_preprocessor.hpp"

#include "core/log.hpp"
#include "core/hash.hpp"
#include "core/string_utils.hpp"
#include "io/file_stream_reader.hpp"
#include "io/file_stream_writer.hpp"

#include <nvrhi/utils.h>

#include <dxc/dxcapi.h>
#include <shaderc/shaderc.hpp>
#include <spirv-tools/libspirv.h>
#include <spirv_cross/spirv_glsl.hpp>

#include <cstdlib>
#include <format>

#include <libshaderc_util/file_finder.h>

#include "vulkan_shader_cache.hpp"
#include "renderer/renderer.hpp"
#include "shader_preprocessing/glsl_includer.hpp"
#include "shader_preprocessing/hlsl_includer.hpp"

namespace tst
{
	static std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderUniformBuffer> > s_UniformBuffers; // set -> binding point -> buffer
	static std::unordered_map<uint32_t, std::unordered_map<uint32_t, ShaderStorageBuffer> > s_StorageBuffers; // set -> binding point -> buffer

	static const char *getCacheDirectory()
	{
		return "res/_cache/shader/vulkan";
	}

	static void createCacheDirectoryIfNeeded()
	{
		String cacheDirectory = getCacheDirectory();
		if (!std::filesystem::exists(cacheDirectory))
			std::filesystem::create_directories(cacheDirectory);
	}

	static EShaderUniformType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
	{
		switch (type.basetype)
		{
			case spirv_cross::SPIRType::Boolean: return EShaderUniformType::eBool;
			case spirv_cross::SPIRType::Int:
				if (type.vecsize == 1)
					return EShaderUniformType::eInt;
				if (type.vecsize == 2)
					return EShaderUniformType::eIVec2;
				if (type.vecsize == 3)
					return EShaderUniformType::eIVec3;
				if (type.vecsize == 4)
					return EShaderUniformType::eIVec4;

			case spirv_cross::SPIRType::UInt: return EShaderUniformType::eUInt;
			case spirv_cross::SPIRType::Float:
				if (type.columns == 3)
					return EShaderUniformType::eMat3;
				if (type.columns == 4)
					return EShaderUniformType::eMat4;

				if (type.vecsize == 1)
					return EShaderUniformType::eFloat;
				if (type.vecsize == 2)
					return EShaderUniformType::eVec2;
				if (type.vecsize == 3)
					return EShaderUniformType::eVec3;
				if (type.vecsize == 4)
					return EShaderUniformType::eVec4;
				break;
		}
		TST_ASSERT(false);
		return {};
	}

	VulkanShaderCompiler::VulkanShaderCompiler(const std::filesystem::path &shaderSourcePath, bool disableOptimization)
		: m_shaderSourcePath(shaderSourcePath), m_disableOptimization(disableOptimization)
	{
		m_Language = shader_utils::shaderLangFromExtension(shaderSourcePath.extension().string());
	}

	bool VulkanShaderCompiler::reload(bool forceCompile)
	{
		m_shaderSource.clear();
		m_StagesMetadata.clear();
		m_SPIRVDebugData.clear();
		m_SPIRVData.clear();

		createCacheDirectoryIfNeeded();
		const String source = tst::utils::readFileAndSkipBOM(m_shaderSourcePath);
		TST_ASSERT(!source.empty());

		TST_TRACE_TAG("Renderer", "Compiling shader: {}", m_shaderSourcePath.string());
		m_shaderSource                        = preProcess(source);
		const nvrhi::ShaderType changedStages = VulkanShaderCache::hasChanged(this);

		bool compileSucceeded = compileOrGetVulkanBinaries(m_SPIRVDebugData, m_SPIRVData, changedStages, forceCompile);
		if (!compileSucceeded)
		{
			TST_ASSERT(false);
			return false;
		}

		// Reflection
		if (forceCompile || changedStages != nvrhi::ShaderType::None || !tryReadCachedReflectionData())
		{
			reflectAllShaderStages(m_SPIRVDebugData);
			serializeReflectionData();
		}

		return true;
	}

	void VulkanShaderCompiler::clearUniformBuffers()
	{
		s_UniformBuffers.clear();
		s_StorageBuffers.clear();
	}

	std::map<nvrhi::ShaderType, String> VulkanShaderCompiler::preProcess(const String &source)
	{
		switch (m_Language)
		{
			case EShaderLanguage::eGLSL: return preProcessGLSL(source);
			case EShaderLanguage::eHLSL: return preProcessHLSL(source);
		}

		TST_ASSERT(false);
		return {};
	}

	std::map<nvrhi::ShaderType, String> VulkanShaderCompiler::preProcessGLSL(const String &source)
	{
		std::map<nvrhi::ShaderType, String> shaderSources = ShaderPreprocessor::preprocessShader<EShaderLanguage::eGLSL>(source, m_AcknowledgedMacros);

		static shaderc::Compiler compiler;

		shaderc_util::FileFinder fileFinder;
		fileFinder.search_path().emplace_back("res/shaders/include/GLSL/");   //Main include directory
		fileFinder.search_path().emplace_back("res/shaders/include/common/"); //Shared include directory
		for (auto &[stage, shaderSource]: shaderSources)
		{
			shaderc::CompileOptions options;
			options.AddMacroDefinition("__GLSL__");
			options.AddMacroDefinition(String(shader_utils::shaderStageToShaderMacro(stage)));

			const auto &globalMacros = Renderer::getGlobalShaderMacros();
			for (const auto &[name, value]: globalMacros)
				options.AddMacroDefinition(name, value);

			// Deleted by shaderc and created per stage
			GlslIncluder *includer = new GlslIncluder(&fileFinder);

			options.SetIncluder(std::unique_ptr<GlslIncluder>(includer));
			const auto preProcessingResult = compiler.PreprocessGlsl(shaderSource, shader_utils::shaderStageToShaderC(stage), m_shaderSourcePath.string().c_str(),
																	 options);
			if (preProcessingResult.GetCompilationStatus() != shaderc_compilation_status_success)
				TST_ERROR_TAG("Renderer",
						  std::format("Failed to pre-process \"{}\"'s {} shader.\nError: {}", m_shaderSourcePath.string(), nvrhi::utils::ShaderStageToString(stage),
							  preProcessingResult.GetErrorMessage()));

			m_StagesMetadata[stage].HashValue = hash::fnvHash(shaderSource);
			m_StagesMetadata[stage].Headers   = std::move(includer->GetIncludeData());

			m_AcknowledgedMacros.merge(includer->GetParsedSpecialMacros());

			shaderSource = String(preProcessingResult.begin(), preProcessingResult.end());
		}
		return shaderSources;
	}

	std::map<nvrhi::ShaderType, String> VulkanShaderCompiler::preProcessHLSL(const String &source)
	{
		std::map<nvrhi::ShaderType, String> shaderSources = ShaderPreprocessor::preprocessShader<EShaderLanguage::eHLSL>(source, m_AcknowledgedMacros);

		#ifdef TST_PLATFORM_WINDOWS
		std::wstring buffer = m_shaderSourcePath.wstring();
		#else
		std::wstring buffer; buffer.resize(m_shaderSourcePath.string().size() * 2); mbstowcs(buffer.data(), m_shaderSourcePath.string().c_str(),
																							 m_shaderSourcePath.string().size());
		#endif

		std::vector<const wchar_t *> arguments{
			buffer.c_str(), L"-P", DXC_ARG_WARNINGS_ARE_ERRORS, L"-I res/shaders/include/common/", L"-I res/shaders/include/HLSL/", //Main include directory
			L"-D", L"__HLSL__",
		};

		const auto &globalMacros = Renderer::getGlobalShaderMacros();
		for (const auto &[name, value]: globalMacros)
		{
			arguments.emplace_back(L"-D");
			arguments.push_back(nullptr);
			String def;
			if (value.size())
				def = std::format("{}={}", name, value);
			else
				def = name;

			wchar_t *def_buffer = new wchar_t[def.size() + 1];
			mbstowcs(def_buffer, def.c_str(), def.size());
			def_buffer[def.size()]          = 0;
			arguments[arguments.size() - 1] = def_buffer;
		}

		if (!DxcInstances::Compiler)
		{
			#ifdef TST_PLATFORM_WINDOWS
			DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&DxcInstances::Compiler));
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&DxcInstances::Utils));
			#endif
		}

		for (auto &[stage, shaderSource]: shaderSources)
		{
			#ifdef TST_PLATFORM_WINDOWS
			IDxcBlobEncoding *pSource;
			DxcInstances::Utils->CreateBlob(shaderSource.c_str(), (uint32_t) shaderSource.size(), CP_UTF8, &pSource);
			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr = pSource->GetBufferPointer();
			sourceBuffer.Size = pSource->GetBufferSize();
			sourceBuffer.Encoding = 0;
			const std::unique_ptr<HlslIncluder> includer = std::make_unique<HlslIncluder>();
			IDxcResult *pCompileResult;
			HRESULT err = DxcInstances::Compiler->Compile(&sourceBuffer, arguments.data(), (uint32_t) arguments.size(), includer.get(), IID_PPV_ARGS(&pCompileResult));

			// Error Handling
			String     error;
			const bool failed = FAILED(err);
			if (failed)
				error = std::format("Failed to pre-process, Error: {}\n", err);
			IDxcBlobEncoding *pErrors = nullptr;
			pCompileResult->GetErrorBuffer(&pErrors);
			if (pErrors->GetBufferPointer() && pErrors->GetBufferSize())
				error.append(std::format("{}\nWhile pre-processing shader file: {} \nAt stage: {}", (char *) pErrors->GetBufferPointer(), m_shaderSourcePath.string(),
										 nvrhi::utils::ShaderStageToString(stage)));
			if (error.empty())
			{
				// Successful compilation
				IDxcBlob *pResult;
				pCompileResult->GetResult(&pResult);

				const size_t size = pResult->GetBufferSize();
				shaderSource      = static_cast<const char *>(pResult->GetBufferPointer());
				pResult->Release();
			}
			else
			{
				TST_ERROR_TAG("Renderer", error);
			}
			m_StagesMetadata[stage].HashValue = hash::fnvHash(shaderSource);
			m_StagesMetadata[stage].Headers   = std::move(includer->getIncludeData());
			m_AcknowledgedMacros.merge(includer->getParsedSpecialMacros());
			#else
			m_StagesMetadata[stage] = StageData{};
			#endif
		}
		return shaderSources;
	}

	String VulkanShaderCompiler::compile(std::vector<uint32_t> &outputBinary, const nvrhi::ShaderType stage, CompilationOptions options) const
	{
		const String &stageSource = m_shaderSource.at(stage);

		if (m_Language == EShaderLanguage::eGLSL)
		{
			static shaderc::Compiler compiler;
			shaderc::CompileOptions  shaderCOptions;
			shaderCOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
			shaderCOptions.SetWarningsAsErrors();
			if (options.GenerateDebugInfo)
				shaderCOptions.SetGenerateDebugInfo();

			if (options.Optimize)
				shaderCOptions.SetOptimizationLevel(shaderc_optimization_level_performance);

			// Compile shader
			const shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(stageSource, shader_utils::shaderStageToShaderC(stage),
																				   m_shaderSourcePath.string().c_str(), shaderCOptions);

			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				return std::format("{}While compiling shader file: {} \nAt stage: {}", module.GetErrorMessage(), m_shaderSourcePath.string(),
								   nvrhi::utils::ShaderStageToString(stage));

			outputBinary = std::vector<uint32_t>(module.begin(), module.end());
			return {}; // Success
		}
		else if (m_Language == EShaderLanguage::eHLSL)
		{
			std::wstring                 buffer = m_shaderSourcePath.wstring();
			std::vector<const wchar_t *> arguments{
				buffer.c_str(), L"-E", L"main", L"-T", shader_utils::HLSLShaderProfile(stage), L"-spirv", L"-fspv-target-env=vulkan1.2", DXC_ARG_PACK_MATRIX_COLUMN_MAJOR,
				DXC_ARG_WARNINGS_ARE_ERRORS
			};

			if (options.GenerateDebugInfo)
			{
				arguments.emplace_back(L"-Qembed_debug");
				arguments.emplace_back(DXC_ARG_DEBUG);
			}

			if ((uint16_t) stage & ((uint16_t) nvrhi::ShaderType::Vertex | (uint16_t) nvrhi::ShaderType::Hull | (uint16_t) nvrhi::ShaderType::Geometry))
				arguments.push_back(L"-fvk-invert-y");

			IDxcBlobEncoding *pSource;
			DxcInstances::Utils->CreateBlob(stageSource.c_str(), (uint32_t) stageSource.size(), CP_UTF8, &pSource);

			DxcBuffer sourceBuffer;
			sourceBuffer.Ptr      = pSource->GetBufferPointer();
			sourceBuffer.Size     = pSource->GetBufferSize();
			sourceBuffer.Encoding = 0;

			IDxcResult *pCompileResult;
			String      error;

			HRESULT err = DxcInstances::Compiler->Compile(&sourceBuffer, arguments.data(), (uint32_t) arguments.size(), nullptr, IID_PPV_ARGS(&pCompileResult));

			// Error Handling
			const bool failed = FAILED(err);
			if (failed)
				error = std::format("Failed to compile, Error: {}\n", err);
			IDxcBlobUtf8 *pErrors;
			pCompileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), NULL);
			if (pErrors && pErrors->GetStringLength() > 0)
				error.append(std::format("{}\nWhile compiling shader file: {} \nAt stage: {}", (char *) pErrors->GetBufferPointer(), m_shaderSourcePath.string(),
										 nvrhi::utils::ShaderStageToString(stage)));

			if (error.empty())
			{
				// Successful compilation
				IDxcBlob *pResult;
				pCompileResult->GetResult(&pResult);

				const size_t size = pResult->GetBufferSize();
				outputBinary.resize(size / sizeof(uint32_t));
				std::memcpy(outputBinary.data(), pResult->GetBufferPointer(), size);
				pResult->Release();
			}
			pCompileResult->Release();
			pSource->Release();

			return error;
		}
		return "Unknown language!";
	}

	RefPtr<VulkanShader> VulkanShaderCompiler::compile(const std::filesystem::path &shaderSourcePath, bool forceCompile, bool disableOptimization)
	{
		// Set name
		String path  = shaderSourcePath.string();
		size_t found = path.find_last_of("/\\");
		String name  = found != String::npos ? path.substr(found + 1) : path;
		found        = name.find_last_of('.');
		name         = found != String::npos ? name.substr(0, found) : name;

		RefPtr<VulkanShader> shader   = make_reference<VulkanShader>();
		shader->m_filePath            = shaderSourcePath;
		shader->m_name                = name;
		shader->m_disableOptimization = disableOptimization;

		RefPtr<VulkanShaderCompiler> compiler = make_reference<VulkanShaderCompiler>(shaderSourcePath);
		compiler->reload(forceCompile);

		shader->loadAndCreateShaders(compiler->getSPIRVData());
		shader->setReflectionData(compiler->m_ReflectionData);
		shader->createDescriptors();

		Renderer::acknowledgeParsedGlobalMacros(compiler->getAcknowledgedMacros(), shader);
		// Renderer::onShaderReloaded(shader->getHash());
		return shader;
	}

	bool VulkanShaderCompiler::recompile(RefPtr<VulkanShader> shader)
	{
		RefPtr<VulkanShaderCompiler> compiler         = make_reference<VulkanShaderCompiler>(shader->m_filePath);
		bool                         compileSucceeded = compiler->reload(true);
		if (!compileSucceeded)
			return false;

		shader->release();

		shader->loadAndCreateShaders(compiler->getSPIRVData());
		shader->setReflectionData(compiler->m_ReflectionData);
		shader->createDescriptors();

		Renderer::acknowledgeParsedGlobalMacros(compiler->getAcknowledgedMacros(), shader);
		// Renderer::onShaderReloaded(shader->getHash());

		return true;
	}

	bool VulkanShaderCompiler::compileOrGetVulkanBinaries(std::map<nvrhi::ShaderType, std::vector<uint32_t> > &outputDebugBinary,
														  std::map<nvrhi::ShaderType, std::vector<uint32_t> > &outputBinary, const nvrhi::ShaderType changedStages,
														  const bool                                           forceCompile)
	{
            for (const auto [stage, _] : m_shaderSource)
		{
			if (!compileOrGetVulkanBinary(stage, outputDebugBinary[stage], true, changedStages, forceCompile))
				return false;
			if (!compileOrGetVulkanBinary(stage, outputBinary[stage], false, changedStages, forceCompile))
				return false;
		}
		return true;
	}

	bool VulkanShaderCompiler::compileOrGetVulkanBinary(nvrhi::ShaderType stage, std::vector<uint32_t> &outputBinary, bool debug, nvrhi::ShaderType changedStages,
														bool              forceCompile)
	{
		const std::filesystem::path cacheDirectory = getCacheDirectory();

		// Compile shader with debug info so we can reflect
		const auto extension = shader_utils::shaderStageCachedFileExtension(stage, debug);
		if (!forceCompile && (uint16_t) stage & ~(uint16_t) changedStages) // Per-stage cache is found and is unchanged
		{
			tryGetVulkanCachedBinary(cacheDirectory, extension, outputBinary);
		}

		if (outputBinary.empty())
		{
			CompilationOptions options;
			if (debug)
			{
				options.GenerateDebugInfo = true;
				options.Optimize          = false;
			}
			else
			{
				options.GenerateDebugInfo = false;
				// Disable optimization for compute shaders because of shaderc internal error
				options.Optimize = !m_disableOptimization && stage != VK_SHADER_STAGE_COMPUTE_BIT;
			}

			if (String error = compile(outputBinary, stage, options); error.size())
			{
				TST_ERROR_TAG("Renderer", "{}", error);
				tryGetVulkanCachedBinary(cacheDirectory, extension, outputBinary);
				if (outputBinary.empty())
				{
					TST_ERROR("Failed to compile shader and couldn't find a cached version.");
				}
				else
				{
					TST_ERROR("Failed to compile {}:{} so a cached version was loaded instead.", m_shaderSourcePath.string(), nvrhi::utils::ShaderStageToString(stage));
				}
				TST_ASSERT(false);
				return false;
			}
			else // Compile success
			{
				auto   path           = cacheDirectory / (m_shaderSourcePath.filename().string() + extension);
				String cachedFilePath = path.string();

				FILE *f = fopen(cachedFilePath.c_str(), "wb");
				if (!f)
					TST_ERROR("Failed to cache shader binary!");
				fwrite(outputBinary.data(), sizeof(uint32_t), outputBinary.size(), f);
				fclose(f);
			}
		}

		return true;
	}

	void VulkanShaderCompiler::clearReflectionData()
	{
		m_ReflectionData.shaderDescriptorSets.clear();
		m_ReflectionData.resources.clear();
		m_ReflectionData.constantBuffers.clear();
		m_ReflectionData.pushConstantRanges.clear();
	}

	void VulkanShaderCompiler::tryGetVulkanCachedBinary(const std::filesystem::path &cacheDirectory, const String &extension, std::vector<uint32_t> &outputBinary) const
	{
		const auto   path           = cacheDirectory / (m_shaderSourcePath.filename().string() + extension);
		const String cachedFilePath = path.string();

		FILE *f = fopen(cachedFilePath.data(), "rb");
		if (!f)
			return;

		fseek(f, 0, SEEK_END);
		uint64_t size = ftell(f);
		fseek(f, 0, SEEK_SET);
		outputBinary = std::vector<uint32_t>(size / sizeof(uint32_t));
		fread(outputBinary.data(), sizeof(uint32_t), outputBinary.size(), f);
		fclose(f);
	}

	bool VulkanShaderCompiler::tryReadCachedReflectionData()
	{
		struct ReflectionFileHeader
		{
			char Header[5] = {'T', 'S', 'T', 'S', 'R'};
		} header;

		std::filesystem::path cacheDirectory = getCacheDirectory();
		const auto            path           = cacheDirectory / (m_shaderSourcePath.filename().string() + ".cached_vulkan.refl");
		FileStreamReader      serializer(path);
		if (!serializer.isStreamGood())
			return false;

		serializer.readRaw(header);

		bool validHeader = memcmp(&header, "TSTSR", 5) == 0;
		TST_ASSERT(validHeader);
		if (!validHeader)
			return false;

		clearReflectionData();

		uint32_t shaderDescriptorSetCount;
		serializer.readRaw<uint32_t>(shaderDescriptorSetCount);

		for (uint32_t i = 0; i < shaderDescriptorSetCount; i++)
		{
			auto &descriptorSet = m_ReflectionData.shaderDescriptorSets.emplace_back();
			serializer.readMap(descriptorSet.uniformBuffers);
			serializer.readMap(descriptorSet.storageBuffers);
			serializer.readMap(descriptorSet.imageSamplers);
			serializer.readMap(descriptorSet.storageImages);
			serializer.readMap(descriptorSet.separateTextures);
			serializer.readMap(descriptorSet.separateSamplers);
			serializer.readMap(descriptorSet.inputDeclarations);
		}

		serializer.readMap(m_ReflectionData.resources);
		serializer.readMap(m_ReflectionData.constantBuffers);
		serializer.readArray(m_ReflectionData.pushConstantRanges);

		return true;
	}

	void VulkanShaderCompiler::serializeReflectionData()
	{
		struct ReflectionFileHeader
		{
			char Header[5] = {'T', 'S', 'T', 'S', 'R'};
		} header;

		std::filesystem::path cacheDirectory = getCacheDirectory();
		const auto            path           = cacheDirectory / (m_shaderSourcePath.filename().string() + ".cached_vulkan.refl");
		FileStreamWriter      serializer(path);
		serializer.writeRaw(header);
		serializeReflectionData(&serializer);
	}

	void VulkanShaderCompiler::serializeReflectionData(StreamWriter *serializer)
	{
		serializer->writeRaw<uint32_t>(static_cast<uint32_t>(m_ReflectionData.shaderDescriptorSets.size()));
		for (const auto &descriptorSet: m_ReflectionData.shaderDescriptorSets)
		{
			serializer->writeMap(descriptorSet.uniformBuffers);
			serializer->writeMap(descriptorSet.storageBuffers);
			serializer->writeMap(descriptorSet.imageSamplers);
			serializer->writeMap(descriptorSet.storageImages);
			serializer->writeMap(descriptorSet.separateTextures);
			serializer->writeMap(descriptorSet.separateSamplers);
			serializer->writeMap(descriptorSet.inputDeclarations);
		}

		serializer->writeMap(m_ReflectionData.resources);
		serializer->writeMap(m_ReflectionData.constantBuffers);
		serializer->writeArray(m_ReflectionData.pushConstantRanges);
	}

	void VulkanShaderCompiler::reflectAllShaderStages(const std::map<nvrhi::ShaderType, std::vector<uint32_t> > &shaderData)
	{
		clearReflectionData();

		for (auto [stage, data]: shaderData)
		{
			reflect(stage, data);
		}
	}

	void VulkanShaderCompiler::reflect(nvrhi::ShaderType shaderStage, const std::vector<uint32_t> &shaderData)
	{
		TST_INFO_TAG("Renderer", "===========================");
		TST_INFO_TAG("Renderer", " Vulkan Shader Reflection");
		TST_INFO_TAG("Renderer", "===========================");
		TST_INFO_TAG("Renderer", m_shaderSourcePath.string());

		spirv_cross::Compiler compiler(shaderData);
		auto                  resources = compiler.get_shader_resources();

		TST_INFO_TAG("Renderer", "Uniform Buffers:");
		for (const auto &resource: resources.uniform_buffers)
		{
			auto activeBuffers = compiler.get_active_buffer_ranges(resource.id);
			// Discard unused buffers from headers
			if (!activeBuffers.empty())
			{
				const auto &name          = resource.name;
				auto &      bufferType    = compiler.get_type(resource.base_type_id);
				int         memberCount   = (int32) bufferType.member_types.size();
				uint32_t    binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t    descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				auto        size          = (uint32_t) compiler.get_declared_struct_size(bufferType);

				if (descriptorSet >= m_ReflectionData.shaderDescriptorSets.size())
					m_ReflectionData.shaderDescriptorSets.resize(descriptorSet + 1);

				ShaderDescriptorSet &shaderDescriptorSet = m_ReflectionData.shaderDescriptorSets[descriptorSet];
				if (s_UniformBuffers[descriptorSet].find(binding) == s_UniformBuffers[descriptorSet].end())
				{
					ShaderUniformBuffer uniformBuffer;
					uniformBuffer.bindingPoint                  = binding;
					uniformBuffer.size                          = size;
					uniformBuffer.name                          = name;
					uniformBuffer.shaderStage                   = nvrhi::ShaderType::All;
					s_UniformBuffers.at(descriptorSet)[binding] = uniformBuffer;
				}
				else
				{
					ShaderUniformBuffer &uniformBuffer = s_UniformBuffers.at(descriptorSet).at(binding);
					if (size > uniformBuffer.size)
						uniformBuffer.size = size;
				}
				shaderDescriptorSet.uniformBuffers[binding] = s_UniformBuffers.at(descriptorSet).at(binding);

				TST_INFO_TAG("Renderer", "  {0} ({1}, {2})", name, descriptorSet, binding);
				TST_INFO_TAG("Renderer", "  Member Count: {0}", memberCount);
				TST_INFO_TAG("Renderer", "  Size: {0}", size);
				TST_INFO_TAG("Renderer", "-------------------");
			}
		}

		TST_INFO_TAG("Renderer", "Storage Buffers:");
		for (const auto &resource: resources.storage_buffers)
		{
			auto activeBuffers = compiler.get_active_buffer_ranges(resource.id);
			// Discard unused buffers from headers
			if (activeBuffers.size())
			{
				const auto &name          = resource.name;
				auto &      bufferType    = compiler.get_type(resource.base_type_id);
				uint32_t    memberCount   = (uint32_t) bufferType.member_types.size();
				uint32_t    binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t    descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t    size          = (uint32_t) compiler.get_declared_struct_size(bufferType);
				bool        readOnly      = compiler.has_decoration(resource.id, spv::DecorationNonWritable);

				if (descriptorSet >= m_ReflectionData.shaderDescriptorSets.size())
					m_ReflectionData.shaderDescriptorSets.resize(descriptorSet + 1);

				ShaderDescriptorSet &shaderDescriptorSet = m_ReflectionData.shaderDescriptorSets[descriptorSet];
				if (s_StorageBuffers[descriptorSet].find(binding) == s_StorageBuffers[descriptorSet].end())
				{
					ShaderStorageBuffer storageBuffer;
					storageBuffer.bindingPoint                  = binding;
					storageBuffer.size                          = size;
					storageBuffer.name                          = name;
					storageBuffer.shaderStage                   = nvrhi::ShaderType::All;
					storageBuffer.readOnly                      = readOnly;
					s_StorageBuffers.at(descriptorSet)[binding] = storageBuffer;
				}
				else
				{
					ShaderStorageBuffer &storageBuffer = s_StorageBuffers.at(descriptorSet).at(binding);
					if (size > storageBuffer.size)
						storageBuffer.size = size;
					// If any stage marks it as non-readonly, it's not readonly
					if (!readOnly)
						storageBuffer.readOnly = false;
				}

				shaderDescriptorSet.storageBuffers[binding] = s_StorageBuffers.at(descriptorSet).at(binding);

				TST_INFO_TAG("Renderer", "  {0} ({1}, {2})", name, descriptorSet, binding);
				TST_INFO_TAG("Renderer", "  Member Count: {0}", memberCount);
				TST_INFO_TAG("Renderer", "  Size: {0}", size);
				TST_INFO_TAG("Renderer", "  ReadOnly: {0}", readOnly);
				TST_INFO_TAG("Renderer", "-------------------");
			}
		}

		TST_INFO_TAG("Renderer", "Push Constant Buffers:");
		for (const auto &resource: resources.push_constant_buffers)
		{
			const auto &bufferName   = resource.name;
			auto &      bufferType   = compiler.get_type(resource.base_type_id);
			auto        bufferSize   = (uint32_t) compiler.get_declared_struct_size(bufferType);
			uint32_t    memberCount  = uint32_t(bufferType.member_types.size());
			uint32_t    bufferOffset = 0;
			if (m_ReflectionData.pushConstantRanges.size())
				bufferOffset = m_ReflectionData.pushConstantRanges.back().offset + m_ReflectionData.pushConstantRanges.back().size;

			auto &pushConstantRange       = m_ReflectionData.pushConstantRanges.emplace_back();
			pushConstantRange.shaderStage = shaderStage;
			pushConstantRange.size        = bufferSize;
			pushConstantRange.offset      = 0; // bufferOffset;

			// Skip empty push constant buffers - these are for the renderer only
			if (bufferName.empty() || bufferName == "u_renderer")
				continue;

			ShaderBuffer &buffer = m_ReflectionData.constantBuffers[bufferName];
			buffer.name          = bufferName;
			buffer.size          = bufferSize;

			TST_INFO_TAG("Renderer", "  Name: {0}", bufferName);
			TST_INFO_TAG("Renderer", "  Member Count: {0}", memberCount);
			TST_INFO_TAG("Renderer", "  Size: {0}", bufferSize);

			for (uint32_t i = 0; i < memberCount; i++)
			{
				auto        type       = compiler.get_type(bufferType.member_types[i]);
				const auto &memberName = compiler.get_member_name(bufferType.self, i);
				auto        size       = (uint32_t) compiler.get_declared_struct_member_size(bufferType, i);
				auto        offset     = compiler.type_struct_member_offset(bufferType, i);

				String uniformName           = std::format("{}.{}", bufferName, memberName);
				buffer.uniforms[uniformName] = ShaderUniform(uniformName, SPIRTypeToShaderUniformType(type), size, offset);
			}
		}

		TST_INFO_TAG("Renderer", "Sampled Images:");
		for (const auto &resource: resources.sampled_images)
		{
			const auto &name          = resource.name;
			auto &      baseType      = compiler.get_type(resource.base_type_id);
			auto &      type          = compiler.get_type(resource.type_id);
			uint32_t    binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t    descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t    dimension     = 0;
			switch (baseType.image.dim)
			{
				case spv::Dim::Dim1D:
					dimension = 1;
					break;
				case spv::Dim::Dim2D:
					dimension = 2;
					break;
				case spv::Dim::Dim3D:
				case spv::Dim::DimCube:
					dimension = 3;
					break;
			}
			uint32_t arraySize = type.array.size() > 0 ? type.array[0] : 1;
			if (arraySize == 0)
				arraySize = 1;
			if (descriptorSet >= m_ReflectionData.shaderDescriptorSets.size())
				m_ReflectionData.shaderDescriptorSets.resize(descriptorSet + 1);

			ShaderDescriptorSet &shaderDescriptorSet = m_ReflectionData.shaderDescriptorSets[descriptorSet];
			auto &               imageSampler        = shaderDescriptorSet.imageSamplers[binding];
			imageSampler.bindingPoint                = binding;
			imageSampler.descriptorSet               = descriptorSet;
			imageSampler.name                        = name;
			imageSampler.shaderStage                 = shaderStage;
			imageSampler.dimension                   = dimension;
			imageSampler.arraySize                   = arraySize;

			m_ReflectionData.resources[name] = ShaderResourceDeclaration(name, descriptorSet, binding, arraySize);

			TST_INFO_TAG("Renderer", "  {0} ({1}, {2})", name, descriptorSet, binding);
		}

		TST_INFO_TAG("Renderer", "Separate Images:");
		for (const auto &resource: resources.separate_images)
		{
			const auto &name          = resource.name;
			auto &      baseType      = compiler.get_type(resource.base_type_id);
			auto &      type          = compiler.get_type(resource.type_id);
			uint32_t    binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t    descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t    dimension     = 0;
			switch (baseType.image.dim)
			{
				case spv::Dim::Dim1D:
					dimension = 1;
					break;
				case spv::Dim::Dim2D:
					dimension = 2;
					break;
				case spv::Dim::Dim3D:
				case spv::Dim::DimCube:
					dimension = 3;
					break;
			}
			uint32_t arraySize = type.array.size() > 0 ? type.array[0] : 1;
			if (arraySize == 0)
				arraySize = 1;
			if (descriptorSet >= m_ReflectionData.shaderDescriptorSets.size())
				m_ReflectionData.shaderDescriptorSets.resize(descriptorSet + 1);

			ShaderDescriptorSet &shaderDescriptorSet = m_ReflectionData.shaderDescriptorSets[descriptorSet];
			auto &               imageSampler        = shaderDescriptorSet.separateTextures[binding];
			imageSampler.bindingPoint                = binding;
			imageSampler.descriptorSet               = descriptorSet;
			imageSampler.name                        = name;
			imageSampler.shaderStage                 = shaderStage;
			imageSampler.dimension                   = dimension;
			imageSampler.arraySize                   = arraySize;

			m_ReflectionData.resources[name] = ShaderResourceDeclaration(name, descriptorSet, binding, arraySize);

			TST_INFO_TAG("Renderer", "  {0} ({1}, {2})", name, descriptorSet, binding);
		}

		TST_INFO_TAG("Renderer", "Separate Samplers:");
		for (const auto &resource: resources.separate_samplers)
		{
			const auto &name          = resource.name;
			auto &      baseType      = compiler.get_type(resource.base_type_id);
			auto &      type          = compiler.get_type(resource.type_id);
			uint32_t    binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t    descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t    dimension     = 0;

			uint32_t arraySize = type.array.size() > 0 ? type.array[0] : 1;
			if (arraySize == 0)
				arraySize = 1;
			if (descriptorSet >= m_ReflectionData.shaderDescriptorSets.size())
				m_ReflectionData.shaderDescriptorSets.resize(descriptorSet + 1);

			ShaderDescriptorSet &shaderDescriptorSet = m_ReflectionData.shaderDescriptorSets[descriptorSet];
			auto &               imageSampler        = shaderDescriptorSet.separateSamplers[binding];
			imageSampler.bindingPoint                = binding;
			imageSampler.descriptorSet               = descriptorSet;
			imageSampler.name                        = name;
			imageSampler.shaderStage                 = shaderStage;
			imageSampler.dimension                   = dimension;
			imageSampler.arraySize                   = arraySize;

			m_ReflectionData.resources[name] = ShaderResourceDeclaration(name, descriptorSet, binding, arraySize);

			TST_INFO_TAG("Renderer", "  {0} ({1}, {2})", name, descriptorSet, binding);
		}

		TST_INFO_TAG("Renderer", "Storage Images:");
		for (const auto &resource: resources.storage_images)
		{
			const auto &name          = resource.name;
			auto &      type          = compiler.get_type(resource.type_id);
			uint32_t    binding       = compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t    descriptorSet = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t    dimension     = 0;
			switch (type.image.dim)
			{
				case spv::Dim::Dim1D:
					dimension = 1;
					break;
				case spv::Dim::Dim2D:
					dimension = 2;
					break;
				case spv::Dim::Dim3D:
				case spv::Dim::DimCube:
					dimension = 3;
					break;
				default: ;
			}
			uint32_t arraySize = !type.array.empty() ? type.array[0] : 1;
			if (arraySize == 0)
				arraySize = 1;
			if (descriptorSet >= m_ReflectionData.shaderDescriptorSets.size())
				m_ReflectionData.shaderDescriptorSets.resize(descriptorSet + 1);

			ShaderDescriptorSet &shaderDescriptorSet = m_ReflectionData.shaderDescriptorSets[descriptorSet];
			auto &               imageSampler        = shaderDescriptorSet.storageImages[binding];
			imageSampler.bindingPoint                = binding;
			imageSampler.descriptorSet               = descriptorSet;
			imageSampler.name                        = name;
			imageSampler.dimension                   = dimension;
			imageSampler.arraySize                   = arraySize;
			imageSampler.shaderStage                 = shaderStage;

			m_ReflectionData.resources[name] = ShaderResourceDeclaration(name, descriptorSet, binding, arraySize);

			TST_INFO_TAG("Renderer", "  {0} ({1}, {2})", name, descriptorSet, binding);
		}

		TST_INFO_TAG("Renderer", "Special macros:");
		for (const auto &macro: m_AcknowledgedMacros)
		{
			TST_INFO_TAG("Renderer", "  {0}", macro);
		}

		TST_INFO_TAG("Renderer", "===========================");
	}
}
