#pragma once

#include "toast_render.hpp"

#include <DirectXMath.h>

#include "toast_gpu/device.hpp"
using namespace DirectX;

namespace toaster::rd
{
	TST_DECLARE_HANDLE(Material);

	struct TST_RENDER_API MaterialData
	{
		static constexpr uint32 maxTextureRefs{4u};

		std::vector<uint8> data;

		std::array<gpu::TextureRef, maxTextureRefs> textureRefs;

		VmaVirtualAllocation virtualAllocation{nullptr};
		uint64               allocationOffset{0u};
		uint64               allocationSize{0u};

		uint32 framesDirty{0u};
	};

	class TST_RENDER_API MaterialSystem
	{
		TST_REGISTER_DEPENDENCY(gpu::Device, Device, device)
	public:
		MaterialSystem(gpu::Device &p_device, uint32 p_max_pool_size_bytes, uint32 p_frames_in_flight = 3u);
		~MaterialSystem();

		MaterialSystem(const MaterialSystem &)            = delete;
		MaterialSystem(MaterialSystem &&)                 = delete;
		MaterialSystem &operator=(const MaterialSystem &) = delete;
		MaterialSystem &operator=(MaterialSystem &&)      = delete;

		[[nodiscard]] auto createMaterial(uint32 p_size_bytes) -> Ref<MaterialSystem, MaterialHandle>;

		auto acquire(MaterialHandle p_handle) -> void { _acquireMaterial(p_handle); }
		auto release(MaterialHandle p_handle) -> void { _releaseMaterial(p_handle); }
		auto isValid(MaterialHandle p_handle) const -> bool { return _isMaterialValid(p_handle); }
		auto getData(MaterialHandle p_handle) const -> const MaterialData * { return _getMaterialData(p_handle); }
		auto getData(MaterialHandle p_handle) -> MaterialData * { return _getMaterialData(p_handle); }

		auto getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress;

		template<typename Type>
		auto setField(MaterialHandle p_handle, uint32 p_byte_offset, const Type &p_data) -> void
		{
			MaterialData *data{_getMaterialData(p_handle)};
			TST_ASSERT(data);
			TST_ASSERT(p_byte_offset + sizeof(Type) <= data->data.size());

			std::memcpy(data->data.data() + p_byte_offset, &p_data, sizeof(Type));
			data->framesDirty = m_framesInFlight;
		}

		// Acquires the requested texture and places at the requested index in the material's texture refs array
		auto setTextureRef(MaterialHandle p_handle, uint32 p_index, const gpu::TextureRef &p_texture) -> void;

		auto update(uint32 p_frame_index) -> void;

	private:
		auto _destroyMaterial(MaterialData *p_data) -> void;

		auto _acquireMaterial(MaterialHandle p_handle) -> void { m_materialPool.incRef(p_handle); }
		auto _releaseMaterial(MaterialHandle p_handle) -> void { _destroyMaterial(m_materialPool.decRef(p_handle)); }
		auto _isMaterialValid(MaterialHandle p_handle) const -> bool { return m_materialPool.isValid(p_handle); }
		auto _getMaterialData(MaterialHandle p_handle) const -> const MaterialData * { return m_materialPool.getData(p_handle); }
		auto _getMaterialData(MaterialHandle p_handle) -> MaterialData * { return m_materialPool.getData(p_handle); }

		Pool<MaterialTag, MaterialData> m_materialPool;
		uint32                          m_maxPoolSize{0u};
		uint32                          m_framesInFlight{3u};
		VmaVirtualBlock                 m_virtualBlock{nullptr};

		std::vector<gpu::BufferRef> m_materialBuffers;
	};

	using MaterialRef = Ref<MaterialSystem, MaterialHandle>;
}
