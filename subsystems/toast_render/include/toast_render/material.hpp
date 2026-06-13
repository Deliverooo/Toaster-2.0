#pragma once

#include "descriptor_set_manager.hpp"

namespace toaster::render
{
	class TST_RENDER_API Material
	{
	public:
		Material(RenderContext &p_render_ctx, const gpu::ShaderHandle &p_shader, const String &p_name = "Unknown?");
		~Material();

		auto setTexture(const String &p_name, const gpu::Texture2DHandle &p_texture_2d) -> void;
		auto setTexture(const String &p_name, const gpu::Texture2DHandle &p_texture_2d, uint32 p_array_index) -> void;

		template<typename Type>
		auto set(const String &p_name, const Type &p_type) -> Material &
		{
			auto decl = _getPushConstantDeclaration(p_name);
			TST_ASSERT_MSG(decl, "Could not find uniform!");
			if (!decl)
				return *this;

			m_pushConstantStorageBuffer.write(&p_type, decl->size, decl->offset);
			return *this;
		}

		auto update(uint32 p_frame_index) -> void;

		auto getDescriptorSet(uint32 p_frame_index) -> vk::DescriptorSet;
		auto hasDescriptorSets() const -> bool;

		template<typename Type>
		auto get(const String &p_name) -> Type &
		{
			auto decl = _getPushConstantDeclaration(p_name);
			TST_ASSERT_MSG(decl, "Could not find uniform!");
			return static_cast<Type &>(m_pushConstantStorageBuffer.readType<Type>(decl->offset));
		}

		template<gpu::GPUResource_c TResource>
		auto getResource(const String &p_name) -> RefPtr<TResource>
		{
			return m_descriptorSetManager->getDescriptor<TResource>(p_name);
		}

		auto getPushConstantStorageBuffer() const -> const Buffer &;

		auto getName() const -> String;

	private:
		auto _getPushConstantDeclaration(const String &p_name) -> const gpu::reflection::PushConstant *;

		NonOwningPtr<RenderContext> m_renderCtx{nullptr};

		gpu::ShaderHandle m_shader{nullptr};
		String            m_name{};

		OwningPtr<DescriptorSetManager> m_descriptorSetManager{nullptr};

		Buffer m_pushConstantStorageBuffer{};
	};

	TST_RENDER_DEFINE_HANDLE(Material, Material)
}
