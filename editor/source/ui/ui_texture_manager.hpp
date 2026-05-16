#pragma once

#include "toast_render/render_context.hpp"

#include <imgui.h>
namespace ig = ImGui;

namespace toaster::ui
{
	class UITextureManager
	{
	public:
		UITextureManager(render::RenderContext *p_render_ctx, vk::DescriptorPool p_descriptor_pool);

		auto registerOrGetTexture(const String &p_name, const gpu::Texture2DHandle &p_texture) -> ImTextureID;
		auto hasTexture(const String &p_name) const -> bool;
		auto getTexture(const String &p_name) const -> ImTextureID;
		auto getTextureSize(const String &p_name) const -> ImVec2;

	private:
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		vk::DescriptorPool            m_descriptorPool{nullptr};
		vk::raii::DescriptorSetLayout m_descriptorSetLayout{nullptr};

		struct TextureInfo
		{
			gpu::Texture2DHandle    textureRef{nullptr};
			vk::raii::DescriptorSet descriptorSet{nullptr};
			ImVec2                  size{0.0f, 0.0f};
		};

		std::unordered_map<String, TextureInfo> m_textureInfoMap;
	};
}
