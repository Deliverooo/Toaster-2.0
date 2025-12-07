#include "hlsl_includer.hpp"

#include "core/hash.hpp"
#include "core/string_utils.hpp"

namespace tst
{
	HRESULT HlslIncluder::LoadSource(LPCWSTR pFilename, IDxcBlob **ppIncludeSource)
	{
		static IDxcUtils *pUtils = nullptr;
		if (!pUtils)
		{
			DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
			HRESULT result = pUtils->CreateDefaultIncludeHandler(&s_DefaultIncludeHandler);
			TST_ASSERT(!FAILED(result));
		}

		const std::filesystem::path filePath                                = pFilename;
		auto &                      [source, sourceHash, stages, isGuarded] = m_HeaderCache[filePath.string()];

		if (source.empty())
		{
			source = utils::readFileAndSkipBOM(filePath);
			if (source.empty())
			{
				// Note(Karim): No error logging because dxc tries multiple include
				// directories with the same file until it finds it.
				*ppIncludeSource = nullptr;
				return S_FALSE;
			}

			sourceHash = hash::fnvHash(source.c_str());

			// Can clear "source" in case it has already been included in this stage.
			stages = ShaderPreprocessor::preprocessHeader<EShaderLanguage::eHLSL>(source, isGuarded, m_ParsedSpecialMacros, m_includeData, filePath);
		}
		else if (isGuarded)
		{
			source.clear();
		}

		//TODO(Karim): Get real values for IncludeDepth and IsRelative?
		m_includeData.emplace(IncludeData{filePath, 0, false, isGuarded, sourceHash, stages});

		IDxcBlobEncoding *pEncoding;
		pUtils->CreateBlob(source.data(), (uint32_t) source.size(), CP_UTF8, &pEncoding);

		*ppIncludeSource = pEncoding;
		return S_OK;
	}
}
