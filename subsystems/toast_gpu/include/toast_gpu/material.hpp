#pragma once

#include <DirectXMath.h>
#include <unordered_map>
#include "buffer.hpp"
#include "texture.hpp"

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
		TST_REGISTER_DEPENDENCY(BufferManager, bufferManager)
		TST_REGISTER_DEPENDENCY(TextureManager, textureManager)
	public:
		MaterialManager(BufferManager &p_buffer_manager, TextureManager &p_texture_manager, const RefPtr<MaterialStructMapping> &p_struct_mapping,
						uint32         p_max_materials);
		~MaterialManager();

		[[nodiscard]] auto createMaterial() -> MaterialHandle;
		auto               destroyMaterial(MaterialHandle p_handle) -> void { m_pool.destroy(p_handle); }

		auto getData(MaterialHandle p_handle) const -> const uint8 * { return m_pool.getData(p_handle)->data; }
		auto getData(MaterialHandle p_handle) -> uint8 * { return m_pool.getData(p_handle)->data; }

		auto getImage(MaterialHandle p_handle, const String &p_name) -> SharedTexture;

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

		auto getSystemBuffer(uint32 p_frame_index) const -> BufferHandle { return m_materialBuffers[p_frame_index]; }

	private:
		auto _set(MaterialHandle p_handle, const String &p_name, const void *p_data) -> void;

		RefPtr<MaterialStructMapping> m_structMapping{nullptr};

		struct MaterialPoolEntry
		{
			std::unordered_map<String, SharedTexture> imageRefs; // Kind of necessary if you don't want weird things to happen
			uint8 *                                   data{nullptr};
			uint32                                    framesDirty{0u};
		};

		Pool<MaterialTag, MaterialPoolEntry> m_pool;

		// All ts is PFF
		std::vector<BufferHandle> m_materialBuffers;

		uint32 m_maxMaterials{0u};
	};

	template<>
	inline auto MaterialManager::set<SharedTexture>(MaterialHandle p_handle, const String &p_name, const SharedTexture &p_data) -> void
	{
		const uint32 heap_slot{m_textureManager->getTextureShaderReadHeapSlot(p_data.get())};
		_set(p_handle, p_name, &heap_slot);
		m_pool._data[p_handle.id].imageRefs[p_name] = p_data;
	}
}
