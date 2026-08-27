#pragma once

#include "device.hpp"
#include "toast_math/math_vector.hpp"

namespace toaster::gpu
{
	// Just all the possible typical permutations of load and store ops
	enum class EAttachmentUsageOP
	{
		eClearStore,
		eLoadStore,
		eNoneStore,
		eDontCareStore,
		eClearNone,
		eLoadNone,
		eNoneNone,
		eDontCareNone,
		eClearDontCare,
		eLoadDontCare,
		eNoneDontCare,
		eDontCareDontCare
	};

	enum class EAttachmentResolveMode : uint8
	{
		eNone,
		eZero,
		eAverage,
		eMin,
		eMax
	};

	struct TST_GPU_API ClearColourValue
	{
		constexpr ClearColourValue() = default;

		constexpr ClearColourValue(float32 p_r, float32 p_g, float32 p_b, float32 p_a) : _float(p_r, p_g, p_b, p_a)
		{
		}

		constexpr ClearColourValue(int32 p_r, int32 p_g, int32 p_b, int32 p_a) : _int(p_r, p_g, p_b, p_a)
		{
		}

		constexpr ClearColourValue(uint32 p_r, uint32 p_g, uint32 p_b, uint32 p_a) : _uint(p_r, p_g, p_b, p_a)
		{
		}

		union
		{
			float32 _float[4u];
			int32   _int[4u];
			uint32  _uint[4u];
		};
	};

	struct TST_GPU_API ClearDepthStencilValue
	{
		constexpr ClearDepthStencilValue() = default;

		constexpr ClearDepthStencilValue(float32 p_depth, uint32 p_stencil) : depth(p_depth), stencil(p_stencil)
		{
		}

		float32 depth{1.0f};
		uint32  stencil{0u};
	};

	union ClearValue
	{
		constexpr ClearValue(const ClearColourValue &p_colour_value = {}) : colour(p_colour_value)
		{
		}

		constexpr ClearValue(const ClearDepthStencilValue &p_clear_depth_stencil_value = {}) : depthStencil(p_clear_depth_stencil_value)
		{
		}

		ClearColourValue       colour;
		ClearDepthStencilValue depthStencil;
	};

	struct TST_GPU_API RenderingAttachmentInfo
	{
		RenderingAttachmentInfo() = default;

		ClearValue clearValue{ClearColourValue{}};

		TextureHandle renderTarget;
		TextureHandle resolveTarget;

		EAttachmentUsageOP     usageOp{EAttachmentUsageOP::eClearStore};
		EAttachmentResolveMode resolveMode{EAttachmentResolveMode::eNone}; // Only use if using multisampling
	};

	struct TST_GPU_API RenderingInfo
	{
		tsm::Rect renderArea{};

		std::vector<RenderingAttachmentInfo>   colourAttachments;
		std::optional<RenderingAttachmentInfo> depthAttachment{std::nullopt};
		std::optional<RenderingAttachmentInfo> stencilAttachment{std::nullopt};
	};

	// To create this, go to Device::createCommandList
	class TST_GPU_API CommandList // You can copy this just like any other object
	{
	public:
		// Easier than managing it in the class itself
		CommandList(Device &p_device, vk::CommandBuffer p_cmd);
		~CommandList();

		CommandList(const CommandList &p_other);
		CommandList(CommandList &&p_other) noexcept;
		CommandList &operator=(const CommandList &p_other);
		CommandList &operator=(CommandList &&p_other) noexcept;

		auto getCommandBuffer() const -> vk::CommandBuffer { return m_cmd; }

		// Only works if the pool was created with the reset bit enabled
		auto reset() -> void;

		auto begin() -> void;
		auto end() -> void;

		auto beginRendering(const RenderingInfo &p_rendering_info) -> void;
		auto endRendering() -> void;

		auto setViewport(const tsm::Viewport &p_viewport) -> void;
		auto setScissor(const tsm::Rect &p_scissor) -> void;

		auto bindResourceHeap(const ResourceDescriptorHeap &p_resource_heap) -> void;
		auto bindSamplerHeap(const SamplerDescriptorHeap &p_sampler_heap) -> void;

		auto bindShaders(const InitialiserList<const ShaderHandle> &p_shaders) -> void;

		auto copyBuffer(BufferHandle p_src_buffer, BufferHandle p_dst_buffer, uint64 p_size, uint64 p_src_offset = 0u, uint64 p_dst_offset = 0u) -> void;
		auto copyBufferToTexture(BufferHandle p_src_buffer, TextureHandle p_dst_texture) -> void;

		auto transitionTextureLayout(TextureHandle p_texture, vk::ImageLayout p_dst_layout) -> void;

	private:
		NonOwningPtr<Device> m_device{nullptr};
		vk::CommandBuffer    m_cmd{nullptr};
	};
}
