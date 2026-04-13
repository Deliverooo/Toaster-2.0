#pragma once

#include <imgui.h>
namespace ig = ImGui;

#include "toaster/toast_lib/string.hpp"

namespace toaster::ui
{
	auto dragFloat(const String &   p_label, float *v, const char *p_id, float p_speed = 1.0f, float p_min = 0.0f, float p_max = 0.0f, const char *p_format = "%.3f",
				   ImGuiSliderFlags p_flags                                            = 0) -> bool;

	auto dragFloatWithReset(const String &p_label, float *p_v, const char *p_id, float p_speed = 1.0f, float p_min = 0.0f, float p_max = 0.0f,
							const char *  p_format = "%.3f", float p_reset_value = 0.0f, ImGuiSliderFlags p_flags = 0) -> bool;

	auto checkbox(const String &p_label, bool *p_v) -> bool;

	auto beginCombo(const String &p_label, const char *preview_value, ImGuiComboFlags flags = 0) -> bool;
	auto endCombo() -> void;

	auto colourEdit3(const String &p_label, float p_col[3], ImGuiColorEditFlags p_flags = 0) -> bool;
	auto colourEdit4(const String &p_label, float p_col[4], ImGuiColorEditFlags p_flags = 0) -> bool;
}
