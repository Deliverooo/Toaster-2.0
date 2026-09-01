#pragma once

#include "toast_render.hpp"

#include <DirectXMath.h>

#include "toast_gpu/device.hpp"
using namespace DirectX;

namespace toaster::rd
{
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

	TST_DECLARE_HANDLE(Material);
	TST_DECLARE_REF(Material);

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

		[[nodiscard]] auto createMaterial(uint32 p_size_bytes) -> MaterialRef;

		auto getMaterialDeviceAddress(MaterialHandle p_handle, uint32 p_frame_index) const -> vk::DeviceAddress;

		template<typename Type>
		auto setField(MaterialHandle p_handle, uint32 p_byte_offset, const Type &p_data) -> void
		{
			MaterialData *data{m_materialResourceManager.getData(p_handle)};
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

		ResourceManager<MaterialTag, MaterialData> m_materialResourceManager;
		uint32                                     m_maxPoolSize{0u};
		uint32                                     m_framesInFlight{3u};
		VmaVirtualBlock                            m_virtualBlock{nullptr};

		std::vector<gpu::BufferRef> m_materialBuffers;
	};
}
