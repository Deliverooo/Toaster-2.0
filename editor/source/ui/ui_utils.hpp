#pragma once

#include <imgui.h>
namespace ig = ImGui;

namespace toaster::ui
{
	class ScopedStyle
	{
	public:
		ScopedStyle(const ScopedStyle &)            = delete;
		ScopedStyle &operator=(const ScopedStyle &) = delete;

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
		ScopedColour(const ScopedColour &)            = delete;
		ScopedColour &operator=(const ScopedColour &) = delete;

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
		ScopedFont(const ScopedFont &)            = delete;
		ScopedFont &operator=(const ScopedFont &) = delete;
		ScopedFont(ImFont *p_font) { ig::PushFont(p_font); }
		~ScopedFont() { ig::PopFont(); }
	};
}
