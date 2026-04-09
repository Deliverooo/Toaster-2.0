#pragma once

#include "vk_shader.hpp"
#include "vk_texture.hpp"
#include "vk_uniform_buffer.hpp"

namespace toaster::gpu
{
	class VKGPUContext;

	enum class EDescriptorType
	{
		eUnknown, eUniformBuffer, eSampler2D
	};

	struct DescriptorDeclaration
	{
		String          name{};
		uint32          set{0u};
		uint32          binding{0u};
		uint32          arraySize{0u};
		EDescriptorType type{EDescriptorType::eUnknown};
	};

	struct DescriptorResource
	{
		std::vector<RefPtr<IGPUResource> > resources;
		EGPUResourceType                   type{EGPUResourceType::eUnknown};

		DescriptorResource() = default;

		DescriptorResource(const RefPtr<VKUniformBuffer> &p_uniform_buffer) : resources(std::vector<RefPtr<IGPUResource> >(1, p_uniform_buffer)),
																			  type(EGPUResourceType::eUniformBuffer)
		{
		}

		DescriptorResource(const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) : resources(std::vector<RefPtr<IGPUResource> >(1, p_uniform_buffer_pff)),
																					 type(EGPUResourceType::eUniformBufferPFF)
		{
		}

		DescriptorResource(const RefPtr<VKTexture2D> &p_texture_2d) : resources(std::vector<RefPtr<IGPUResource> >(1, p_texture_2d)), type(EGPUResourceType::eTexture2D)
		{
		}
	};

	class VKDescriptorSetManager
	{
	public:
		VKDescriptorSetManager(VKGPUContext *p_ctx, const RefPtr<VKShader> &p_shader, uint32 p_start_set, uint32 p_end_set);

		void setDescriptor(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer);
		void setDescriptor(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff);
		void setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d);
		void setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index);

		template<GPUResource_c TResource>
		RefPtr<TResource> getDescriptor(const String &p_name)
		{
			if (const auto decl{getDescriptorDeclaration(p_name)})
				if (const auto set_it{m_descriptorResources.find(decl->set)}; set_it != m_descriptorResources.end())
					if (const auto resource_it{set_it->second.find(decl->binding)}; resource_it != set_it->second.end())
						return resource_it->second.resources[0].as<TResource>();
			return nullptr;
		}

		// Only call when you have set all your required descriptors :)
		void bakeDescriptors();

		void updateDescriptors(uint32 p_frame_index);

		[[nodiscard]] std::vector<vk::DescriptorSet> getDescriptorSets(uint32 p_frame_index) const;

		const DescriptorDeclaration *                            getDescriptorDeclaration(const String &p_name) const;
		const std::unordered_map<String, DescriptorDeclaration> &getDescriptorDeclarations() const;

		const RefPtr<VKTexture2D> &getWhiteTexture() const;

		uint32 getStartSetIndex() const;
		uint32 getEndSetIndex() const;

	private:
		EDescriptorType  _getDescriptorType(vk::DescriptorType p_type) const;
		EGPUResourceType _getResourceType(vk::DescriptorType p_type) const;

		VKGPUContext *m_ctx{nullptr};

		RefPtr<VKShader> m_shader{nullptr};
		uint32           m_startSet{0u};
		uint32           m_endSet{3u};

		vk::raii::DescriptorPool m_descriptorPool{nullptr};

		struct WriteDescriptor
		{
			vk::WriteDescriptorSet wds{};
			std::vector<void *>    resourceHandles;
		};

		std::vector<std::unordered_map<uint32, std::unordered_map<uint32, WriteDescriptor> > > m_writeDescriptorMap;
		std::unordered_map<String, DescriptorDeclaration>                                      m_descriptorDeclarations;

		std::unordered_map<uint32, std::unordered_map<uint32, DescriptorResource> > m_descriptorResources;
		std::unordered_map<uint32, std::unordered_map<uint32, DescriptorResource> > m_invalidDescriptorResources;

		std::vector<std::vector<vk::raii::DescriptorSet> > m_descriptorSets;

		RefPtr<VKTexture2D> m_whiteTexture{nullptr};
	};
}
