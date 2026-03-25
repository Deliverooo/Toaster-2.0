#include "ui_widgets.hpp"

#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_lib/toast_assert.h"

namespace toaster::ui
{
	constexpr float text_padding = 5.0f;

	bool dragFloat(const String &p_label, float *v, const char *p_id, float p_speed, float p_min, float p_max, const char *p_format, ImGuiSliderFlags p_flags)
	{
		TST_ASSERT(v);

		ImGuiStyle &style = ig::GetStyle();
		ImGuiIO &   io    = ig::GetIO();

		ig::PushID(p_id);
		ImVec2 content_region = ig::GetContentRegionAvail();
		float  slider_width   = (content_region.x / 2.0f) - (2.0f * style.FramePadding.x);

		float right_side_padding = ig::GetFrameHeight();

		float center_offset = (content_region.x / 6.0f);

		float prev_cursor_pos = ig::GetCursorPosX();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - ig::CalcTextSize(p_label.c_str()).x - style.FramePadding.x - text_padding - center_offset);
		ig::Text("%s", p_label.c_str());
		ig::SameLine();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - center_offset);
		ig::PushItemWidth(slider_width - right_side_padding + center_offset);
		bool result = ig::DragFloat(p_id, v, p_speed, p_min, p_max, p_format, p_flags);
		ig::PopItemWidth();

		ig::PopID();

		return result;
	}

	bool dragFloatWithReset(const String &   p_label, float *p_v, const char *p_id, float p_speed, float p_min, float p_max, const char *p_format, float p_reset_value,
							ImGuiSliderFlags p_flags)
	{
		TST_ASSERT(p_v);

		ImGuiStyle &style = ig::GetStyle();
		ImGuiIO &   io    = ig::GetIO();

		ig::PushID(p_id);
		float button_width       = ig::GetFrameHeight();
		float right_side_padding = button_width;

		ImVec2 content_region = ig::GetContentRegionAvail();
		float  slider_width   = (content_region.x / 2.0f) - (2.0f * style.FramePadding.x);

		float center_offset = (content_region.x / 6.0f);

		float prev_cursor_pos = ig::GetCursorPosX();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - ig::CalcTextSize(p_label.c_str()).x - style.FramePadding.x - text_padding - center_offset);
		ig::Text("%s", p_label.c_str());
		ig::SameLine();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - center_offset);
		ig::PushItemWidth(slider_width - right_side_padding + center_offset);
		bool result = ig::DragFloat(p_id, p_v, p_speed, p_min, p_max, p_format, p_flags);
		ig::PopItemWidth();

		ig::SameLine();

		ig::PushFont(io.Fonts->Fonts[1]);
		if (ig::Button("r", ImVec2{button_width, button_width}))
			*p_v = p_reset_value;
		ig::PopFont();

		ig::PopID();

		return result;
	}

	bool checkbox(const String &p_label, bool *p_v)
	{
		TST_ASSERT(p_v);

		ImVec2 content_region = ig::GetContentRegionAvail();
		float  button_offset  = (content_region.x / 2.0f);

		float center_offset = (content_region.x / 6.0f);

		float prev_cursor_pos = ig::GetCursorPosX();

		ig::SetCursorPosX(prev_cursor_pos + button_offset - center_offset);
		bool result = ig::Checkbox(p_label.c_str(), p_v);

		return result;
	}

	bool beginCombo(const String &p_label, const char *preview_value, ImGuiComboFlags flags)
	{
		ImGuiStyle &style = ig::GetStyle();
		ImGuiIO &   io    = ig::GetIO();

		ImVec2 content_region = ig::GetContentRegionAvail();
		float  slider_width   = (content_region.x / 2.0f) - (2.0f * style.FramePadding.x);

		float right_side_padding = ig::GetFrameHeight();

		float center_offset = (content_region.x / 6.0f);

		float prev_cursor_pos = ig::GetCursorPosX();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - ig::CalcTextSize(p_label.c_str()).x - style.FramePadding.x - text_padding - center_offset);
		ig::Text("%s", p_label.c_str());
		ig::SameLine();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - center_offset);
		ig::SetNextItemWidth(slider_width - right_side_padding + center_offset);
		// ig::PushItemWidth(slider_width - right_side_padding + center_offset);
		bool result = ig::BeginCombo("##", preview_value, flags);
		// ig::PopItemWidth();

		return result;
	}

	void endCombo()
	{
		ig::EndCombo();
	}

	bool colourEdit3(const String &p_label, float p_col[3], ImGuiColorEditFlags p_flags)
	{
		ImGuiStyle &style = ig::GetStyle();
		ImGuiIO &   io    = ig::GetIO();

		ImVec2 content_region = ig::GetContentRegionAvail();
		float  slider_width   = (content_region.x / 2.0f) - (2.0f * style.FramePadding.x);

		float right_side_padding = ig::GetFrameHeight();

		float center_offset = (content_region.x / 6.0f);

		float prev_cursor_pos = ig::GetCursorPosX();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - ig::CalcTextSize(p_label.c_str()).x - style.FramePadding.x - text_padding - center_offset);
		ig::Text("%s", p_label.c_str());
		ig::SameLine();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - center_offset);
		ig::SetNextItemWidth(slider_width - right_side_padding + center_offset);
		bool result = ig::ColorEdit3("##", p_col, p_flags);

		return result;
	}

	bool colourEdit4(const String &p_label, float p_col[4], ImGuiColorEditFlags p_flags)
	{
		ImGuiStyle &style = ig::GetStyle();
		ImGuiIO &   io    = ig::GetIO();

		ImVec2 content_region = ig::GetContentRegionAvail();
		float  slider_width   = (content_region.x / 2.0f) - (2.0f * style.FramePadding.x);

		float right_side_padding = ig::GetFrameHeight();

		float center_offset = (content_region.x / 6.0f);

		float prev_cursor_pos = ig::GetCursorPosX();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - ig::CalcTextSize(p_label.c_str()).x - style.FramePadding.x - text_padding - center_offset);
		ig::Text("%s", p_label.c_str());
		ig::SameLine();

		ig::SetCursorPosX(prev_cursor_pos + (content_region.x / 2.0f) - center_offset);
		ig::SetNextItemWidth(slider_width - right_side_padding + center_offset);
		bool result = ig::ColorEdit4("##", p_col, p_flags);

		return result;
	}
}
