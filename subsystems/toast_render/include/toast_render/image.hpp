#pragma once

#include "toast_render.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_raw_image.hpp"

namespace toaster::render
{
	class RenderContext;

	using ImageSize = tsm::uint2;

	struct TST_RENDER_API ImageSpecInfo
	{
		ImageSize size{0u};

		vk::Format          format{vk::Format::eR8G8B8A8Srgb};
		vk::ImageUsageFlags usageFlags{vk::ImageUsageFlagBits::eSampled};

		uint32 layerCount{1u};
	};

	class TST_RENDER_API Image
	{
		TST_RENDER_OBJECT
	public:
		Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info);
		Image(RenderContext &p_render_ctx, const ImageSpecInfo &p_spec_info, const Buffer &p_data);
		~Image();

		auto setData(const Buffer &p_data) -> void;

		auto getSpecInfo() const -> const ImageSpecInfo &;
		auto getImage() const -> const gpu::RawImageHandle &;
		auto getImage() -> gpu::RawImageHandle &;

		auto resize(tsm::uint2 p_size) -> void;

		auto setShaderRead() -> void;

		auto getHeapID() const -> gpu::DescriptorSlot;
		auto getAlignedHeapID() const -> gpu::DescriptorSlot;

	private:
		ImageSpecInfo m_specInfo{};

		gpu::RawImageHandle m_image{nullptr};

		gpu::DescriptorSlot m_heapID{UINT32_MAX};
	};

	TST_RENDER_DEFINE_HANDLE(Image, Image)
}
