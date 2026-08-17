#pragma once

#include <unordered_map>

#include "allocator.hpp"
#include "toast_lib/pool.hpp"

#include <DirectXMath.h>

namespace toaster::gpu
{
	struct MaterialStructMember
	{
		String name;
		uint32 offset{0u};
		uint32 size{0u};
	};

	// This could be thought of as the material version of vertex attributes... :)
	struct MaterialStructMapping
	{
		uint32                                           size{0u};
		std::unordered_map<String, MaterialStructMember> members;
	};

	TST_DECLARE_HANDLE(Material);

	// Manages a pool of materials from an arbitrary struct declaration. Ideally you would obtain this from shader reflection,
	// but it can easily be specified manually
	class TST_GPU_API MaterialManager // TODO: Specify frame in flight count
	{
	public:
		MaterialManager(const LogicalDevice &p_device, Allocator &p_allocator, const RefPtr<MaterialStructMapping> &p_struct_mapping, uint32 p_max_materials);
		~MaterialManager();

		[[nodiscard]] auto createMaterial() -> MaterialHandle;
		auto               destroyMaterial(MaterialHandle p_handle) -> void;

		auto isValid(MaterialHandle p_handle) const -> bool;
		auto getData(MaterialHandle p_handle) -> void *;

		template<typename Type>
		auto set(MaterialHandle p_handle, const String &p_name, const Type &p_data) -> void
		{
			_set(p_handle, p_name, &p_data);
		}

		// Silly DirectX...
		auto XM_CALLCONV set(TST_UNUSED MaterialHandle p_handle, TST_UNUSED const String &p_name, TST_UNUSED DirectX::XMVECTOR p_data) -> void
		{
			TST_PERMA_ASSERT_MSG(false, "XMMATRIX is an ambiguous type. Store in a XMFLOATXXX to get the actual size of the required type");
		}

		auto XM_CALLCONV set(TST_UNUSED MaterialHandle p_handle,TST_UNUSED const String &p_name,TST_UNUSED DirectX::XMMATRIX p_data) -> void
		{
			TST_PERMA_ASSERT_MSG(false, "XMVECTOR is an ambiguous type. Store in a XMFLOATX to get the actual size of the required type");
		}

		// I don't know why you would call this, but you can... Maybe if you manually memcpy outside this system ? :)
		auto markMaterialDirty(MaterialHandle p_handle) -> void;

		auto update(uint32 p_frame_index) -> void;

		auto getSystemDeviceAddress(uint32 p_frame_index) const -> vk::DeviceAddress { return m_materialBufferDeviceAddresses[p_frame_index]; }

	private:
		auto _set(MaterialHandle p_handle, const String &p_name, const void *p_data) -> void;

		NonOwningPtr<Allocator> m_allocator{nullptr};

		RefPtr<MaterialStructMapping> m_structMapping{nullptr};

		struct Entry
		{
			uint8 *data{nullptr};
			uint32 framesDirty{0u};
		};

		Pool<MaterialTag, Entry> m_pool;

		// All ts is PFF
		std::vector<vk::Buffer>        m_materialBuffers;
		std::vector<VmaAllocation>     m_materialBufferAllocations;
		std::vector<uint8 *>           m_mappedMaterialBuffers;
		std::vector<vk::DeviceAddress> m_materialBufferDeviceAddresses;

		uint32 m_maxMaterials{0u};
	};
}
