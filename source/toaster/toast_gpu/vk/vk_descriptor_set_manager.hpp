#pragma once

#include "vk_shader.hpp"
#include "vk_storage_buffer.hpp"
#include "vk_storage_image.hpp"
#include "vk_texture.hpp"
#include "vk_uniform_buffer.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;

	enum class EDescriptorType
	{
		eUnknown,
		eUniformBuffer,
		eStorageBuffer,
		eSampler2D,
		eSampler3D,
		eImage2D,
		eImage3D
	};

	struct DescriptorDeclaration
	{
		String          name{};
		uint32          set{0u};
		uint32          binding{0u};
		uint32          arraySize{0u};
		EDescriptorType type{EDescriptorType::eUnknown};
	};

	struct TST_GPU_API DescriptorResource
	{
		std::vector<GPUResourceHandle> resources;
		EGPUResourceType               type{EGPUResourceType::eUnknown};

		DescriptorResource() = default;

		template<GPUResource_c TResource>
		DescriptorResource(const RefPtr<TResource> &p_resource) : resources(std::vector<GPUResourceHandle>(1, p_resource.template as<IGPUResource>())),
																  type(p_resource->getResourceType())
		{
		}

		// You should only use this for texture 2ds!
		template<GPUResource_c TResource>
		auto set(const RefPtr<TResource> &p_resource, uint32 p_index) -> void
		{
			type               = p_resource->getResourceType();
			resources[p_index] = p_resource.template as<IGPUResource>(); // Workaround to prevent wierd const related compile error
		}
	};

	class TST_GPU_API VKDescriptorSetManager
	{
		TST_GPU_OBJECT
	public:
		VKDescriptorSetManager(VKLogicalDevice *p_device, const ShaderHandle &p_shader, uint32 p_start_set, uint32 p_end_set);

		template<GPUResource_c TResource>
		auto setDescriptor(const String &p_name, const RefPtr<TResource> &p_resource, uint32 p_array_index = UINT32_MAX) -> void
		{
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

		auto getStartSetIndex() const -> uint32;
		auto getEndSetIndex() const -> uint32;

	private:
		static auto _getDescriptorType(vk::DescriptorType p_type) -> EDescriptorType;
		static auto _getResourceType(vk::DescriptorType p_type) -> EGPUResourceType;

		ShaderHandle m_shader{nullptr};
		uint32       m_startSet{0u};
		uint32       m_endSet{3u};

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

		Texture2DHandle m_whiteTexture{nullptr};
		Texture3DHandle m_whiteTexture3D{nullptr};
	};
}
