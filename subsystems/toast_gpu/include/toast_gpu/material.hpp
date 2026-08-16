#pragma once

#include <unordered_map>

#include "descriptor_heap.hpp"

namespace toaster::gpu
{
	struct MaterialStructMember
	{
		String name;
		uint32 offset{0u};
		uint32 size{0u};
	};

	struct MaterialStructMapping
	{
		uint32                                           size{0u};
		std::unordered_map<String, MaterialStructMember> members;
	};

	class TST_GPU_API Material
	{
	public:
		Material(LogicalDevice &p_device, Allocator &p_allocator, ResourceDescriptorHeap &p_resource_heap, const RefPtr<MaterialStructMapping> &p_struct_mapping);
		~Material();

		auto getHeapID(uint32 p_frame_index) const -> uint32 { return m_materialHeapIDs[p_frame_index]; }

		template<typename Type>
		auto set(const String &p_name, const Type &p_value) -> void
		{
			_set(p_name, static_cast<const void *>(&p_value));
		}

		auto update(uint32 p_frame_index) -> void;

	private:
		auto _set(const String &p_name, const void *p_value) -> void;

		NonOwningPtr<Allocator>              m_allocator{nullptr};
		NonOwningPtr<ResourceDescriptorHeap> m_resourceDescriptorHeap{nullptr};

		RefPtr<MaterialStructMapping> m_structMapping{nullptr};

		uint8 *m_materialData{nullptr};

		std::vector<vk::Buffer>    m_materialBuffers;
		std::vector<VmaAllocation> m_materialBufferAllocations;
		std::vector<void *>        m_mappedMaterialBuffers;

		std::vector<uint32> m_materialHeapIDs;

		uint32 m_numFramesDirty{0u};
	};
}
