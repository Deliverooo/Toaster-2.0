#pragma once

#include <DirectXMath.h>

#include "device.hpp"
using namespace DirectX;

#include "shader_manager.hpp"

namespace toaster::gpu
{
	TST_DECLARE_HANDLE(Material);

	struct TST_GPU_API MaterialData
	{
		static constexpr uint32 maxTextureRefs{4u};

		std::vector<uint8> data;

		std::array<TextureHandle, maxTextureRefs> textureRefs;

		VmaVirtualAllocation virtualAllocation{nullptr};
		uint64       allocationOffset{0u};
		uint64       allocationSize{0u};

		uint32 framesDirty{0u};

		template<typename Type>
		auto setMaterialParameter(uint32 p_byte_offset, const Type &p_data) -> void // Useful for shared materials to use
		{
			TST_PERMA_ASSERT(p_byte_offset + sizeof(Type) <= data.size());

			std::memcpy(data.data() + p_byte_offset, &p_data, sizeof(Type));
			framesDirty = 3u;
		}
	};

	class TST_GPU_API MaterialManager
	{
		TST_REGISTER_DEPENDENCY(Device, Device, device)
	public:
		using PoolType = Pool<MaterialTag, MaterialData>;

		MaterialManager(Device &p_device, uint32 p_max_pool_size_bytes);
		~MaterialManager();

		MaterialManager(const MaterialManager &)            = delete;
		MaterialManager(MaterialManager &&)                 = delete;
		MaterialManager &operator=(const MaterialManager &) = delete;
		MaterialManager &operator=(MaterialManager &&)      = delete;

		[[nodiscard]] auto createMaterial(uint32 p_size_bytes) -> MaterialHandle;
		auto			   acquireMaterial(MaterialHandle p_handle) -> void { m_materialPool.incRef(p_handle);}
		auto               releaseMaterial(MaterialHandle p_handle) -> void { _destroyMaterial(m_materialPool.decRef(p_handle)); }
		auto               isMaterialValid(MaterialHandle p_handle) const -> bool { return m_materialPool.isValid(p_handle); }

		auto getMaterialData(MaterialHandle p_handle) const -> const MaterialData * { return m_materialPool.getData(p_handle); }
		auto getMaterialData(MaterialHandle p_handle) -> MaterialData * { return m_materialPool.getData(p_handle); }

		auto getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress;

		template<typename Type>
		auto setMaterialParameter(MaterialHandle p_handle, uint32 p_byte_offset, const Type &p_data) -> void
		{
			MaterialData *data{getMaterialData(p_handle)};
			TST_ASSERT(data);
			TST_ASSERT(p_byte_offset + sizeof(Type) <= data->data.size());

			std::memcpy(data->data.data() + p_byte_offset, &p_data, sizeof(Type));
			data->framesDirty = 3u;
		}

		// Acquires the requested texture and places at the requested index in the material's texture refs array
		auto setTextureRef(MaterialHandle p_handle, uint32 p_index, TextureHandle p_texture) -> void;

		auto update(uint32 p_frame_index) -> void;

	private:
		auto _destroyMaterial(MaterialData *p_data) -> void;

		Pool<MaterialTag, MaterialData> m_materialPool;
		uint32                          m_maxPoolSize{0u};
		VmaVirtualBlock                 m_virtualBlock{nullptr};

		std::vector<BufferHandle> m_materialBuffers;
	};
}
