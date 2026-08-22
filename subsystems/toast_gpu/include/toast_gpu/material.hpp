#pragma once

#include <DirectXMath.h>
#include <unordered_map>
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace toaster::gpu
{
	TST_DECLARE_HANDLE(Material);

	struct TST_GPU_API MaterialData
	{
		static constexpr uint32 maxTextureRefs{4u};

		// TODO: Render state / pipeline thing...
		std::vector<uint8> data;

		std::array<SharedTexture, maxTextureRefs> textureRefs;

		uint32 allocationOffset{0u};
		uint32 allocationSize{0u};

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
		TST_REGISTER_DEPENDENCY(BufferManager, BufferManager, bufferManager)
	public:
		using DestroyCallback = void(*)(void *, MaterialHandle);
		using PoolType        = Pool<MaterialTag, MaterialData>;

		MaterialManager(BufferManager &p_buffer_manager, uint32 p_max_pool_size_bytes, void *p_user_data = nullptr, const DestroyCallback &p_destroy_callback = nullptr);
		~MaterialManager();

		auto setUserData(void *p_user_data) -> void { m_userData = p_user_data; }
		auto setDestroyCallback(const DestroyCallback &p_destroy_callback) -> void { m_destroyCallback = p_destroy_callback; }

		[[nodiscard]] auto createMaterial(/*RenderStateHandle TODO*/ uint32 p_size_bytes) -> MaterialHandle;
		[[nodiscard]] auto createSharedMaterial(uint32 p_size_bytes) -> SharedMaterial;
		auto               destroyMaterial(MaterialHandle p_handle) -> void { m_pool.destroy(p_handle); }

		// Used for external callers to fall back to the default destruction logic, so DON'T use outside of callbacks
		auto destroyData(MaterialData *p_data) -> void;

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
		Pool<MaterialTag, MaterialData> m_pool;
		uint32                          m_maxPoolSize{0u};

		std::vector<BufferHandle> m_materialBuffers;
		uint32                    m_currentAllocationOffset{0u};

		void *          m_userData{nullptr};
		DestroyCallback m_destroyCallback{nullptr};
	};
}
