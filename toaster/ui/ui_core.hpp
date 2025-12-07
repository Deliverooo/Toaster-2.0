#pragma once

#include "gpu/texture.hpp"

#include <imgui.h>

namespace tst::ui
{
	ImTextureID getTextureID(RefPtr<Image2D> image);
	ImTextureID getTextureID(RefPtr<Texture2D> texture);

	void image(const RefPtr<Image2D> &image, const ImVec2 &size, const ImVec2 &uv0 = ImVec2(0, 0), const ImVec2 &      uv1        = ImVec2(1, 1),
			   const ImVec4 &         tint_col                                     = ImVec4(1, 1, 1, 1), const ImVec4 &border_col = ImVec4(0, 0, 0, 0));
	void image(const RefPtr<Image2D> &image, uint32_t layer, const ImVec2 &size, const ImVec2 &uv0 = ImVec2(0, 0), const ImVec2 &      uv1        = ImVec2(1, 1),
			   const ImVec4 &         tint_col                                                     = ImVec4(1, 1, 1, 1), const ImVec4 &border_col = ImVec4(0, 0, 0, 0));
	void imageMip(const RefPtr<Image2D> &image, uint32_t mip, const ImVec2 &size, const ImVec2 &uv0 = ImVec2(0, 0), const ImVec2 &      uv1        = ImVec2(1, 1),
				  const ImVec4 &         tint_col                                                   = ImVec4(1, 1, 1, 1), const ImVec4 &border_col = ImVec4(0, 0, 0, 0));
	void image(const RefPtr<Texture2D> &texture, const ImVec2 &size, const ImVec2 &uv0 = ImVec2(0, 0), const ImVec2 &      uv1        = ImVec2(1, 1),
			   const ImVec4 &           tint_col                                       = ImVec4(1, 1, 1, 1), const ImVec4 &border_col = ImVec4(0, 0, 0, 0));
}
