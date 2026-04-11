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
		VKGPUContext *getContext() const;

		void set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d);
		void set(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index);

		template<typename Type>
		void set(const String &p_name, const Type &p_type)
		{
			auto decl = _getPushConstantDeclaration(p_name);
			TST_ASSERT_MSG(decl, "Could not find uniform!");
			if (!decl)
				return;

			m_pushConstantStorageBuffer.write(&p_type, decl->size, decl->offset);
		}

		void update(uint32 p_frame_index);

		vk::DescriptorSet getDescriptorSet(uint32 p_frame_index);
		bool              hasDescriptorSets() const;

		template<GPUResource_c TResource>
		RefPtr<TResource> get(const String &p_name)
		{
			return m_descriptorSetManager->getDescriptor<TResource>(p_name);
		}

		const Buffer &getPushConstantStorageBuffer() const;

	private:
		const PushConstant *_getPushConstantDeclaration(const String &p_name);

		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};

		UniquePtr<VKDescriptorSetManager> m_descriptorSetManager{nullptr};

		Buffer m_pushConstantStorageBuffer{};
	};
}
