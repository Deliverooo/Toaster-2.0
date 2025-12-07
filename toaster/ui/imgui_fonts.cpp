#include "imgui_fonts.hpp"

#include <unordered_map>

#include "error_macros.hpp"
#include "core/log.hpp"

namespace tst::ui
{
	static std::unordered_map<String, ImFont *> s_Fonts;

	void Fonts::add(const FontConfiguration &config, bool isDefault)
	{
		if (s_Fonts.find(config.FontName) != s_Fonts.end())
		{
			TST_WARN_TAG("EditorUI", "Tried to add font with name '{0}' but that name is already taken!", config.FontName);
			return;
		}

		ImFontConfig imguiFontConfig;
		imguiFontConfig.MergeMode = config.MergeWithLast;
		auto &  io                = ImGui::GetIO();
		ImFont *font              = io.Fonts->AddFontFromFileTTF(config.FilePath.data(), config.Size, &imguiFontConfig,
																 config.GlyphRanges == nullptr ? io.Fonts->GetGlyphRangesDefault() : config.GlyphRanges);
		TST_ASSERT(font);
		s_Fonts[config.FontName] = font;

		if (isDefault)
			io.FontDefault = font;
		io.FontDefault = font;
	}

	ImFont *Fonts::get(const String &fontName)
	{
		TST_ASSERT(s_Fonts.find(fontName) != s_Fonts.end());
		return s_Fonts.at(fontName);
	}

	void Fonts::pushFont(const String &fontName)
	{
		const auto &io = ImGui::GetIO();

		if (s_Fonts.find(fontName) == s_Fonts.end())
		{
			ImGui::PushFont(io.FontDefault);
			return;
		}

		ImGui::PushFont(s_Fonts.at(fontName));
	}

	void Fonts::popFont()
	{
		ImGui::PopFont();
	}
}
