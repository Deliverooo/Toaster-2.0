#pragma once

#include "vk_shader.hpp"
#include "vk_storage_buffer.hpp"
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
		std::vector<const IGPUResource *> resources;
		EGPUResourceType                  type{EGPUResourceType::eUnknown};

		DescriptorResource() = default;

		DescriptorResource(const RefPtr<VKUniformBuffer> &p_uniform_buffer) : resources(std::vector<const IGPUResource *>(1, p_uniform_buffer.get())),
																			  type(EGPUResourceType::eUniformBuffer)
		{
		}

		DescriptorResource(const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) : resources(std::vector<const IGPUResource *>(1, p_uniform_buffer_pff.get())),
																					 type(EGPUResourceType::eUniformBufferPFF)
		{
		}

		DescriptorResource(const RefPtr<VKStorageBuffer> &p_storage_buffer) : resources(std::vector<const IGPUResource *>(1, p_storage_buffer.get())),
																			  type(EGPUResourceType::eStorageBuffer)
		{
		}

		DescriptorResource(const RefPtr<VKStorageBufferPFF> &p_storage_buffer_pff) : resources(std::vector<const IGPUResource *>(1, p_storage_buffer_pff.get())),
																					 type(EGPUResourceType::eStorageBufferPFF)
		{
		}

		DescriptorResource(const RefPtr<VKTexture2D> &p_texture_2d) : resources(std::vector<const IGPUResource *>(1, p_texture_2d.get())),
																	  type(EGPUResourceType::eTexture2D)
		{
		}

		DescriptorResource(const RefPtr<VKImage2D> &p_image_2d) : resources(std::vector<const IGPUResource *>(1, p_image_2d.get())), type(EGPUResourceType::eImage2D)
		{
		}

		DescriptorResource(const RefPtr<VKTexture3D> &p_texture_3d) : resources(std::vector<const IGPUResource *>(1, p_texture_3d.get())),
																	  type(EGPUResourceType::eTexture3D)
		{
		}

		auto set(const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_index) -> void
		{
			type               = EGPUResourceType::eTexture2D;
			resources[p_index] = p_texture_2d.as<IGPUResource>().get();
		}

		DescriptorResource(const VKUniformBuffer *p_uniform_buffer) : resources(std::vector<const IGPUResource *>(1, p_uniform_buffer)),
																	  type(EGPUResourceType::eUniformBuffer)
		{
		}

		DescriptorResource(const VKUniformBufferPFF *p_uniform_buffer_pff) : resources(std::vector<const IGPUResource *>(1, p_uniform_buffer_pff)),
																			 type(EGPUResourceType::eUniformBufferPFF)
		{
		}

		DescriptorResource(const VKStorageBuffer *p_storage_buffer) : resources(std::vector<const IGPUResource *>(1, p_storage_buffer)),
																	  type(EGPUResourceType::eStorageBuffer)
		{
		}

		DescriptorResource(const VKStorageBufferPFF *p_storage_buffer_pff) : resources(std::vector<const IGPUResource *>(1, p_storage_buffer_pff)),
																			 type(EGPUResourceType::eStorageBufferPFF)
		{
		}

		DescriptorResource(const VKTexture2D *p_texture_2d) : resources(std::vector<const IGPUResource *>(1, p_texture_2d)), type(EGPUResourceType::eTexture2D)
		{
		}

		DescriptorResource(const VKImage2D *p_image_2d) : resources(std::vector<const IGPUResource *>(1, p_image_2d)), type(EGPUResourceType::eImage2D)
		{
		}

		DescriptorResource(const VKTexture3D *p_texture_3d) : resources(std::vector<const IGPUResource *>(1, p_texture_3d)), type(EGPUResourceType::eTexture3D)
		{
		}

		auto set(const VKTexture2D *p_texture_2d, uint32 p_index) -> void
		{
			type               = EGPUResourceType::eTexture2D;
			resources[p_index] = dynamic_cast<const IGPUResource *>(p_texture_2d);
		}
	};

	class TST_GPU_API VKDescriptorSetManager
	{
		TST_GPU_OBJECT
	public:
		VKDescriptorSetManager(VKLogicalDevice *p_device, const RefPtr<VKShader> &p_shader, uint32 p_start_set, uint32 p_end_set);

		auto setDescriptor(const String &p_name, const VKUniformBuffer *p_uniform_buffer) -> void;
		auto setDescriptor(const String &p_name, const VKUniformBufferPFF *p_uniform_buffer_pff) -> void;
		auto setDescriptor(const String &p_name, const VKStorageBuffer *p_storage_buffer) -> void;
		auto setDescriptor(const String &p_name, const VKStorageBufferPFF *p_storage_buffer_pff) -> void;
		auto setDescriptor(const String &p_name, const VKTexture2D *p_texture_2d) -> void;
		auto setDescriptor(const String &p_name, const VKTexture2D *p_texture_2d, uint32 p_array_index) -> void;
		auto setDescriptor(const String &p_name, const VKImage2D *p_image_2d) -> void;
		auto setDescriptor(const String &p_name, const VKTexture3D *p_texture_3d) -> void;

		auto setDescriptor(const String &p_name, const RefPtr<VKUniformBuffer> &p_uniform_buffer) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKUniformBufferPFF> &p_uniform_buffer_pff) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKStorageBuffer> &p_storage_buffer) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKStorageBufferPFF> &p_storage_buffer_pff) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKTexture2D> &p_texture_2d, uint32 p_array_index) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKImage2D> &p_image_2d) -> void;
		auto setDescriptor(const String &p_name, const RefPtr<VKTexture3D> &p_texture_3d) -> void;

		template<GPUResource_c TResource>
		auto getDescriptor(const String &p_name) -> TResource *
		{
			if (const auto decl{getDescriptorDeclaration(p_name)})
				if (const auto set_it{m_descriptorResources.find(decl->set)}; set_it != m_descriptorResources.end())
					if (const auto resource_it{set_it->second.find(decl->binding)}; resource_it != set_it->second.end())
						return dynamic_cast<TResource *>(resource_it->second.resources[0]);
			return nullptr;
		}

		// Only call when you have set all your required descriptors :)
		auto bakeDescriptors() -> void;
		auto updateDescriptors(uint32 p_frame_index) -> void;

		[[nodiscard]] auto getDescriptorSets(uint32 p_frame_index) const -> std::vector<vk::DescriptorSet>;

		auto getDescriptorDeclaration(const String &p_name) const -> const DescriptorDeclaration *;
		auto getDescriptorDeclarations() const -> const std::unordered_map<String, DescriptorDeclaration> &;

		auto getWhiteTexture() const -> const VKTexture2D *;
		auto getWhiteTexture3D() const -> const VKTexture3D *;

		auto hasDescriptorSets() const -> bool;

		auto getStartSetIndex() const -> uint32;
		auto getEndSetIndex() const -> uint32;

	private:
		static auto _getDescriptorType(vk::DescriptorType p_type) -> EDescriptorType;
		static auto _getResourceType(vk::DescriptorType p_type) -> EGPUResourceType;

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

		UniquePtr<VKTexture2D> m_whiteTexture{nullptr};
		UniquePtr<VKTexture3D> m_whiteTexture3D{nullptr};
	};
}
