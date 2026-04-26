#pragma once

#include <imgui.h>
namespace ig = ImGui;

namespace toaster::ui
{
	class ScopedStyle
	{
	public:
		ScopedStyle(const ScopedStyle &)                     = delete;
		auto operator=(const ScopedStyle &) -> ScopedStyle & = delete;

		template<typename Type>
		ScopedStyle(ImGuiStyleVar p_style_var, Type p_value)
		{
			ig::PushStyleVar(p_style_var, p_value);
		}

		~ScopedStyle() { ig::PopStyleVar(); }
	};

	class ScopedColour
	{
	public:
		ScopedColour(const ScopedColour &)                     = delete;
		auto operator=(const ScopedColour &) -> ScopedColour & = delete;

		template<typename Type>
		ScopedColour(ImGuiCol p_colour_id, Type p_colour)
		{
			ig::PushStyleColor(p_colour_id, ImColor(p_colour).Value);
		}

		~ScopedColour() { ig::PopStyleColor(); }
	};

	class ScopedFont
	{
	public:
		ScopedFont(const ScopedFont &)                     = delete;
		auto operator=(const ScopedFont &) -> ScopedFont & = delete;
		ScopedFont(ImFont *p_font) { ig::PushFont(p_font); }
		~ScopedFont() { ig::PopFont(); }
	};
}
