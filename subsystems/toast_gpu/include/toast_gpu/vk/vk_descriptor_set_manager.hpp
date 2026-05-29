#pragma once

#include "vk_common.hpp"
#include "vk_shader.hpp"
#include "vk_storage_buffer.hpp"
#include "vk_storage_image.hpp"
#include "vk_texture.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	struct DescriptorDeclaration
	{
		String          name{};
		uint32          set{0u};
		uint32          binding{0u};
		uint32          arraySize{0u};
		EDescriptorType type{EDescriptorType::eUnknown};
	};

	struct DescriptorSetManagerSpecInfo
	{
		ShaderHandle shader{nullptr};
		uint32       startSet{0u};
		uint32       endSet{3u};
	};

	class TST_GPU_API VKDescriptorSetManager
	{
		TST_GPU_OBJECT
	public:
		VKDescriptorSetManager(VKLogicalDevice *p_device, const DescriptorSetManagerSpecInfo &p_spec_info);

		template<GPUResource_c TResource>
		auto setDescriptor(const String &p_name, const RefPtr<TResource> &p_resource, uint32 p_array_index = UINT32_MAX) -> void
		{
			TST_ASSERT_MSG(p_resource, "If you are going to pass a null resource, then don't pass any resource at all. It will be resolved automatically");
			const auto decl{getDescriptorDeclaration(p_name)};
			TST_ASSERT_MSG((p_array_index == UINT32_MAX) ||(p_array_index < decl->arraySize), "Out of bounds");

			if (decl)
			{
				if (p_array_index == UINT32_MAX)
					m_descriptorResources.at(decl->set)[decl->binding] = p_resource;
				else
					m_descriptorResources.at(decl->set)[decl->binding].set(p_resource, p_array_index);
			}
			else
				LOG_WARN("Descriptor was not found: {}", p_name);
		}

		template<GPUResource_c TResource>
		auto getDescriptor(const String &p_name) -> RefPtr<TResource>
		{
			if (const auto decl{getDescriptorDeclaration(p_name)})
				if (const auto set_it{m_descriptorResources.find(decl->set)}; set_it != m_descriptorResources.end())
					if (const auto resource_it{set_it->second.find(decl->binding)}; resource_it != set_it->second.end())
						return resource_it->second.resources[0].as<TResource>();
			return nullptr;
		}

		// Only call when you have set all your required descriptors :)
		auto bakeDescriptors() -> void;
		auto updateDescriptors(uint32 p_frame_index) -> void;

		[[nodiscard]] auto getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>;

		auto getDescriptorDeclaration(const String &p_name) const -> const DescriptorDeclaration *;
		auto getDescriptorDeclarations() const -> const std::unordered_map<String, DescriptorDeclaration> &;

		auto getWhiteTexture() const -> const Texture2DHandle &;
		auto getWhiteTexture3D() const -> const Texture3DHandle &;

		auto hasDescriptorSets() const -> bool;

		auto getSpecInfo() const -> const DescriptorSetManagerSpecInfo &;

	private:
		static auto _populateWriteDescriptorTexture2DArray(WriteDescriptor &p_write_descriptor, const DescriptorResource &p_resource,
														   std::vector<std::vector<vk::DescriptorImageInfo> > &p_descriptor_image_infos,
														   uint32 &p_descriptor_image_info_index, uint32 p_frame_index) -> void;

		DescriptorSetManagerSpecInfo m_specInfo{};

		vk::raii::DescriptorPool m_descriptorPool{nullptr};

		std::unordered_map<String, DescriptorDeclaration>  m_descriptorDeclarations;
		PerFrameVec<SetMap<BindingMap<WriteDescriptor> > > m_writeDescriptorMap;

		SetMap<BindingMap<DescriptorResource> > m_descriptorResources;
		SetMap<BindingMap<DescriptorResource> > m_invalidDescriptorResources;

		// Each frame has multiple descriptor sets equal to the amount used in the shader
		PerFrameVec<std::vector<vk::raii::DescriptorSet> > m_descriptorSets;

		Texture2DHandle m_whiteTexture{nullptr};
		Texture3DHandle m_whiteTexture3D{nullptr};
	};
}
