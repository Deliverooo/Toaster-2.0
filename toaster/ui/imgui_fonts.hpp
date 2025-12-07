#pragma once

#include <imgui.h>

#include "core/string.hpp"

namespace tst::ui
{
	struct FontConfiguration
	{
		String         FontName;
		StringView     FilePath;
		float          Size          = 16.0f;
		const ImWchar *GlyphRanges   = nullptr;
		bool           MergeWithLast = false;
	};

	class Fonts
	{
	public:
		static void    add(const FontConfiguration &config, bool isDefault = false);
		static void    pushFont(const String &fontName);
		static void    popFont();
		static ImFont *get(const String &fontName);
	};
}
