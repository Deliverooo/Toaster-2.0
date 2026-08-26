#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#include "buffer_manager.hpp"
#include "shader_manager.hpp"
#include "texture_manager.hpp"

namespace toaster::gpu
{
	TST_DECLARE_HANDLE(Material);

	struct TST_GPU_API MaterialData
	{
		static constexpr uint32 maxTextureRefs{4u};

		// TODO: Render state / pipeline thing...
		std::vector<uint8> data;

		std::array<SharedTexture, maxTextureRefs> textureRefs;

		VmaVirtualAllocation virtualAllocation{nullptr};
		vk::DeviceSize       allocationOffset{0u};
		vk::DeviceSize       allocationSize{0u};

		uint32 framesDirty{0u};

		template<typename Type>
		auto setMaterialParameter(uint32 p_byte_offset, const Type &p_data) -> void // Useful for shared materials to use
		{
			TST_PERMA_ASSERT(p_byte_offset + sizeof(Type) <= data.size());

			std::memcpy(data.data() + p_byte_offset, &p_data, sizeof(Type));
			framesDirty = 3u;
		}
	};

	using SharedMaterial = SharedHandle<MaterialTag, MaterialData>;

	class TST_GPU_API MaterialManager
	{
		TST_REGISTER_DEPENDENCY(Device, Device, gpuCtx)
		TST_REGISTER_DEPENDENCY(BufferManager, BufferManager, bufferManager)
	public:
		using PoolType = Pool<MaterialTag, MaterialData>;

		MaterialManager(Device &p_gpu_ctx, BufferManager &p_buffer_manager, uint32 p_max_pool_size_bytes);
		~MaterialManager();

		MaterialManager(const MaterialManager &)            = delete;
		MaterialManager(MaterialManager &&)                 = delete;
		MaterialManager &operator=(const MaterialManager &) = delete;
		MaterialManager &operator=(MaterialManager &&)      = delete;

		[[nodiscard]] auto createMaterial(/*RenderStateHandle TODO*/ uint32 p_size_bytes) -> MaterialHandle;
		[[nodiscard]] auto createSharedMaterial(uint32 p_size_bytes) -> SharedMaterial;
		auto               destroyMaterial(MaterialHandle p_handle) -> void { m_pool.destroy(p_handle); }

		auto getData(MaterialHandle p_handle) const -> const MaterialData * { return m_pool.getData(p_handle); }
		auto getData(MaterialHandle p_handle) -> MaterialData * { return m_pool.getData(p_handle); }

		auto getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress;

		template<typename Type>
		auto setMaterialParameter(MaterialHandle p_handle, uint32 p_byte_offset, const Type &p_data) -> void
		{
			MaterialData *data{getData(p_handle)};
			TST_PERMA_ASSERT(data);
			TST_PERMA_ASSERT(p_byte_offset + sizeof(Type) <= data->data.size());

			std::memcpy(data->data.data() + p_byte_offset, &p_data, sizeof(Type));
			data->framesDirty = 3u;
		}

		auto setTextureRef(MaterialHandle p_handle, uint32 p_index, const SharedTexture &p_texture) -> void;

		auto update(uint32 p_frame_index) -> void;

	private:
		// All the required dependencies to destroy a material. May be called after this class is destroyed due to the deletion queue
		static auto _destroyMaterialData(const MaterialData &p_material_data, VmaVirtualBlock p_virtual_block) -> void;

		Pool<MaterialTag, MaterialData> m_pool;
		uint32                          m_maxPoolSize{0u};
		VmaVirtualBlock                 m_virtualBlock{nullptr};

		std::vector<BufferHandle> m_materialBuffers;
	};
}
