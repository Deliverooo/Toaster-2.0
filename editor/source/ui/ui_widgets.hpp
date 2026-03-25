#pragma once

#include <imgui.h>
namespace ig = ImGui;

#include "toaster/toast_lib/string.hpp"

namespace toaster::ui
{
	bool dragFloat(const String &   p_label, float *v, const char *p_id, float p_speed = 1.0f, float p_min = 0.0f, float p_max = 0.0f, const char *p_format = "%.3f",
				   ImGuiSliderFlags p_flags                                            = 0);

	bool dragFloatWithReset(const String &p_label, float *p_v, const char *p_id, float p_speed = 1.0f, float   p_min         = 0.0f, float            p_max   = 0.0f,
							const char *  p_format                                             = "%.3f", float p_reset_value = 0.0f, ImGuiSliderFlags p_flags = 0);

	bool checkbox(const String &p_label, bool *p_v);

	bool beginCombo(const String &p_label, const char *preview_value, ImGuiComboFlags flags = 0);
	void endCombo();

	bool colourEdit3(const String &p_label, float p_col[3], ImGuiColorEditFlags p_flags = 0);
	bool colourEdit4(const String &p_label, float p_col[4], ImGuiColorEditFlags p_flags = 0);
}
