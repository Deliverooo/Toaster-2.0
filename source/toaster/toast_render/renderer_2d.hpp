#pragma once

#include "../toaster_macros.hpp"

#include "toast_gpu/buffer_layout.hpp"
#include "toast_lib/ptr.hpp"
#include "toast_lib/math/math_vector.hpp"

#include <array>
#include <vulkan/vulkan_raii.hpp>

#include "material.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

namespace toaster::gpu
{
	class VKLogicalDevice;
	class VKPipeline;
	class VKRenderPass;
	class VKMaterial;
	class VKTexture2D;
	class VKRawImage;
	class VKUniformBuffer;
	class VKUniformBufferPFF;
	class VKVertexBuffer;
	class VKIndexBuffer;
	class VKShader;
}

namespace toaster::render
{
	class RenderContext;

	struct TST_API Renderer2DSpecInfo
	{
		uint32 renderTargetWidth{1920u};
		uint32 renderTargetHeight{1080u};

		uint32 maxQuads{10000u};

		bool overrideAttachments{false};
	};

	class TST_API Renderer2D final
	{
	public:
		struct Stats
		{
			uint32 quadCount{0u};
		};

		explicit Renderer2D(RenderContext *p_render_ctx, const Renderer2DSpecInfo &p_create_info);
		~Renderer2D();

		auto begin(uint32 p_frame_index, const tsm::float4x4 &p_view_matrix, const tsm::float4x4 &p_proj_matrix) -> void;
		auto end(gpu::VKCommandBuffer *        p_cmd, uint32 p_frame_index, gpu::RenderingAttachmentInfo *p_override_colour_attachment = nullptr, const gpu::RenderingAttachmentInfo *p_override_depth_attachment                                                             = nullptr) -> void;

		auto submitQuad(const tsm::float3 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float2 &p_position, const tsm::float2 &p_scale, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float4x4 &p_transform, const tsm::float4 &p_colour) -> void;
		auto submitQuad(const tsm::float4x4 &p_transform, const gpu::Texture2DHandle &p_texture, const tsm::float4 &p_colour) -> void;

		auto onResize(uint32 p_width, uint32 p_height) -> void;

		auto               getOutputColourTexture() const -> const gpu::Texture2DHandle &;
		[[nodiscard]] auto getStats() const -> const Stats &;

	private:
		auto _beginNewBatch() -> void;
		auto _getTextureSlotIndex(const gpu::Texture2DHandle &p_texture) -> uint32;

		NonOwningPtr<RenderContext> m_renderContext{nullptr};

		Renderer2DSpecInfo m_createInfo;
		uint32             m_maxVertices;
		uint32             m_maxIndices;

		struct QuadVertex
		{
			tsm::float4 position{0.0f};
			tsm::float4 colour{1.0f};
			tsm::float2 texCoord{0.0f};
			float32     texIndex{0u};
			float32     tilingFactor{1.0f};
		};

		gpu::BufferLayout m_quadVertexBufferLayout;

		gpu::Texture2DHandle m_renderTargetTexture{nullptr};
		gpu::RawImageHandle  m_renderTargetDepthImage{nullptr};

		gpu::PipelineHandle   m_quadPipeline{nullptr};
		gpu::RenderPassHandle m_quadRenderPass{nullptr};
		MaterialHandle        m_quadMaterial{nullptr};

		gpu::VertexBufferHandle m_quadVertexBuffer{nullptr};
		gpu::IndexBufferHandle  m_quadIndexBuffer{nullptr};

		QuadVertex *m_quadVertexBase{nullptr};
		QuadVertex *m_quadVertexPtr{nullptr};

		uint32 m_quadIndexCount{0u};

		std::array<tsm::float4, 4u> m_quadVertexPositions{};
		std::array<tsm::float2, 4u> m_quadVertexTexCoords{};

		struct CameraUB
		{
			glm::mat4 view;
			glm::mat4 proj;
		};

		RefPtr<gpu::VKUniformBufferPFF> m_cameraUBs{nullptr};
		std::vector<void *>             m_mappedCameraUBs;

		std::array<gpu::Texture2DHandle, 32u> m_textureSlots;
		uint32                                m_textureSlotIndex{1u};

		Stats m_stats;
	};
}
