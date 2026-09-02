#pragma once

#include "device.hpp"

namespace toaster::gpu
{
	struct TST_GPU_API ResourceManagerDesc
	{
		// For the resource and sampler heaps, these define the maximum number of descriptors that can allocated of the specified type
		uint32 maxBufferDescriptors{32u};
		uint32 maxImageDescriptors{32u};
		uint32 maxSamplerDescriptors{8u}; // You probably don't need that many samplers (Don't have one per image!)
	};

	class TST_GPU_API ResourceManager
	{
		TST_REGISTER_DEPENDENCY(Device, Device, device)
	public:
		ResourceManager(Device &p_device, const ResourceManagerDesc &p_desc);
		~ResourceManager();

		auto getResourceHeap() const -> ResourceDescriptorHeap & { return *m_resourceHeap; }
		auto getSamplerHeap() const -> SamplerDescriptorHeap & { return *m_samplerHeap; }

		// So far, when creating textures and buffers, all descriptor heap writes have been queued. When the time is appropriate, call this to flush and write them
		auto updateResourceDescriptorWrites() -> void;
		auto updateSamplerDescriptorWrites() -> void;

		[[nodiscard]] auto createBuffer(const BufferDesc &p_desc) -> BufferHandle;
		[[nodiscard]] auto createTexture(const TextureDesc &p_desc) -> TextureHandle;
		[[nodiscard]] auto createSampler(const SamplerDesc &p_desc) -> SamplerHandle;
		[[nodiscard]] auto createShader(const ShaderDesc &p_desc) -> ShaderHandle;

		auto destroyBuffer(BufferHandle p_handle) -> void;
		auto destroyTexture(TextureHandle p_handle) -> void;
		auto destroySampler(SamplerHandle p_handle) -> void;
		auto destroyShader(ShaderHandle p_handle) -> void;

		auto isBufferValid(BufferHandle p_handle) const -> bool { return m_bufferPool.isValid(p_handle); }
		auto isTextureValid(TextureHandle p_handle) const -> bool { return m_texturePool.isValid(p_handle); }
		auto isSamplerValid(SamplerHandle p_handle) const -> bool { return m_samplerPool.isValid(p_handle); }
		auto isShaderValid(ShaderHandle p_handle) const -> bool { return m_shaderPool.isValid(p_handle); }

		auto getBufferData(BufferHandle p_handle) -> BufferData * { return m_bufferPool.getData(p_handle); }
		auto getTextureData(TextureHandle p_handle) -> TextureData * { return m_texturePool.getData(p_handle); }
		auto getSamplerData(SamplerHandle p_handle) -> SamplerData * { return m_samplerPool.getData(p_handle); }
		auto getShaderData(ShaderHandle p_handle) -> ShaderData * { return m_shaderPool.getData(p_handle); }
		auto getBufferData(BufferHandle p_handle) const -> const BufferData * { return m_bufferPool.getData(p_handle); }
		auto getTextureData(TextureHandle p_handle) const -> const TextureData * { return m_texturePool.getData(p_handle); }
		auto getSamplerData(SamplerHandle p_handle) const -> const SamplerData * { return m_samplerPool.getData(p_handle); }
		auto getShaderData(ShaderHandle p_handle) const -> const ShaderData * { return m_shaderPool.getData(p_handle); }

		// Passed to the resource pool in the deferred deletion struct as the target value for a deletion
		auto setGlobalTimelineValue(uint64 p_timeline_value) -> void { m_currentTimelineValue = p_timeline_value; }
		auto getGlobalTimelineValue() const -> uint64 { return m_currentTimelineValue; }

		auto performGarbageCollection(uint64 p_current_timeline_value) -> void;

		#pragma region resource specific non-command list operations

		auto uploadBufferData(BufferHandle p_handle, const void *p_data, uint64 p_size, uint64 p_offset = 0u) -> void;

		auto getTextureShaderReadHeapSlot(TextureHandle p_handle) const -> uint32;
		auto getTextureStorageHeapSlot(TextureHandle p_handle) const -> uint32;

		#pragma endregion

	private:
		auto _destroyBuffer(BufferData &p_data) -> void;
		auto _destroyTexture(TextureData &p_data) -> void;
		auto _destroySampler(SamplerData &p_data) -> void;
		auto _destroyShader(ShaderData &p_data) -> void;

		ResourcePool<BufferTag, BufferData>   m_bufferPool;
		ResourcePool<TextureTag, TextureData> m_texturePool;
		ResourcePool<SamplerTag, SamplerData> m_samplerPool;
		ResourcePool<ShaderTag, ShaderData>   m_shaderPool;

		UniquePtr<ResourceDescriptorHeap> m_resourceHeap{nullptr};
		UniquePtr<SamplerDescriptorHeap>  m_samplerHeap{nullptr};

		uint64 m_currentTimelineValue{0u};
	};
}
