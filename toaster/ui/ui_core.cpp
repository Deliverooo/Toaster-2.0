#include "ui_core.hpp"

namespace tst::ui
{
	ImTextureID getTextureID(RefPtr<Image2D> image)
	{
		return (ImTextureID) image->getHandle().Get();
	}

	ImTextureID getTextureID(RefPtr<Texture2D> texture)
	{
		return getTextureID(texture->GetImage());
	}

	ImTextureID getTextureIDLayer(RefPtr<Image2D> image, uint32_t imageLayer)
	{
		return (ImTextureID) image->getHandle().Get();
	}

	ImTextureID getTextureIDMip(RefPtr<Image2D> image, uint32_t mip)
	{
		return (ImTextureID) image->getHandle().Get();
	}

	void image(const RefPtr<Image2D> &image, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col)
	{
		TST_ASSERT(image);

		const auto textureID = getTextureID(image);
		ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
	}

	void image(const RefPtr<Image2D> &image, uint32_t imageLayer, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col,
			   const ImVec4 &         border_col)
	{
		TST_ASSERT(image);

		const auto textureID = getTextureIDLayer(image, imageLayer);
		ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
	}

	void imageMip(const RefPtr<Image2D> &image, uint32_t mip, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col)
	{
		TST_ASSERT(image);

		const auto textureID = getTextureIDMip(image, mip);
		ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
	}

	void image(const RefPtr<Texture2D> &texture, const ImVec2 &size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col)
	{
		TST_ASSERT(texture);

		const auto textureID = getTextureID(texture->GetImage());
		ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
	}
}
