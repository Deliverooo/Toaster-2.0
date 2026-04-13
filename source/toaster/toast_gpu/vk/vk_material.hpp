#pragma once

#include "vk_descriptor_set_manager.hpp"
#include "vk_shader.hpp"
#include "toast_lib/core_basic.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	class VKMaterial
	{
	public:
		VKMaterial(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader);
		~VKMaterial();
		auto getContext() const -> VKGPUContext *;

		auto set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void;
		auto set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index) -> void;

		template<typename Type>
		auto set(const String &p_name, const Type &p_type) -> void
		{
			auto decl = _getPushConstantDeclaration(p_name);
			TST_ASSERT_MSG(decl, "Could not find uniform!");
			if (!decl)
				return;

			m_pushConstantStorageBuffer.write(&p_type, decl->size, decl->offset);
		}

		auto update(uint32 p_frame_index) -> void;

		auto getDescriptorSet(uint32 p_frame_index) -> vk::DescriptorSet;
		auto hasDescriptorSets() const -> bool;

		template<typename Type>
		auto get(const String &p_name) -> Type &
		{
			auto decl = _getPushConstantDeclaration(p_name);
			TST_ASSERT_MSG(decl, "Could not find uniform!");
			return (Type&)m_pushConstantStorageBuffer.read<Type>(decl->offset);
		}

		template<GPUResource_c TResource>
		auto getResource(const String &p_name) -> RefPtr<TResource>
		{
			return m_descriptorSetManager->getDescriptor<TResource>(p_name);
		}

		auto getPushConstantStorageBuffer() const -> const Buffer &;

	private:
		auto _getPushConstantDeclaration(const String &p_name) -> const PushConstant *;

		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};

		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};

		Buffer m_pushConstantStorageBuffer{};
	};
}
