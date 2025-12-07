#pragma once

#include <glm/glm.hpp>

#include "gpu/render_pass.hpp"
#include "gpu/texture.hpp"
#include "gpu/command_buffer.hpp"
#include "gpu/index_buffer.hpp"
#include "gpu/material.hpp"
#include "gpu/uniform_buffer_set.hpp"
#include "gpu/vertex_buffer.hpp"

namespace tst
{
	struct Renderer2DSpecInfo
	{
		bool     SwapChainTarget = false;
		uint32_t MaxQuads        = 5000;
		uint32_t MaxLines        = 1000;
	};

	class Renderer2D : public RefCounted
	{
	public:
		Renderer2D(const Renderer2DSpecInfo &specification = Renderer2DSpecInfo());
		virtual ~Renderer2D();

		void Init();
		void Shutdown();

		void BeginScene(const glm::mat4 &viewProj, const glm::mat4 &view, bool depthTest = true);
		void EndScene();
		
		// New: Allow external command buffer for direct swapchain rendering
		void BeginScene(RefPtr<CommandBuffer> commandBuffer, const glm::mat4 &viewProj, const glm::mat4 &view, bool depthTest = true);
		void EndScene(RefPtr<CommandBuffer> commandBuffer);

		RefPtr<RenderPass> GetTargetRenderPass();
		void               SetTargetFramebuffer(RefPtr<Framebuffer> framebuffer);

		void OnRecreateSwapchain();

		// Primitives
		void DrawQuad(const glm::mat4 &transform, const glm::vec4 &color);
		void DrawQuad(const glm::mat4 &transform, const RefPtr<Texture2D> &texture, float tilingFactor = 1.0f, const glm::vec4 &    tintColor = glm::vec4(1.0f),
					  glm::vec2        uv0                                                             = glm::vec2(0.0f), glm::vec2 uv1       = glm::vec2(1.0f));

		void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);
		void DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
		void DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const RefPtr<Texture2D> &texture, float tilingFactor = 1.0f,
					  const glm::vec4 &tintColor = glm::vec4(1.0f), glm::vec2 uv0 = glm::vec2(0.0f), glm::vec2 uv1 = glm::vec2(1.0f));
		void DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const RefPtr<Texture2D> &texture, float tilingFactor = 1.0f,
					  const glm::vec4 &tintColor = glm::vec4(1.0f), glm::vec2 uv0 = glm::vec2(0.0f), glm::vec2 uv1 = glm::vec2(1.0f));

		void DrawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
		void DrawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size, const RefPtr<Texture2D> &texture, float tilingFactor = 1.0f,
							   const glm::vec4 &tintColor                                                                             = glm::vec4(1.0f));

		void DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color);
		void DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color);
		void DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const RefPtr<Texture2D> &texture, float tilingFactor = 1.0f,
							 const glm::vec4 &tintColor                                                                                             = glm::vec4(1.0f));
		void DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const RefPtr<Texture2D> &texture, float tilingFactor = 1.0f,
							 const glm::vec4 &tintColor                                                                                             = glm::vec4(1.0f));

		void DrawRotatedRect(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color, const bool onTop = false);
		void DrawRotatedRect(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color, const bool onTop = false);

		// Thickness is between 0 and 1
		void DrawCircle(const glm::vec3 &p0, const glm::vec3 &rotation, float radius, const glm::vec4 &color, const bool onTop = false);
		void DrawCircle(const glm::mat4 &transform, const glm::vec4 &color, bool const onTop = false);
		void FillCircle(const glm::vec2 &p0, float radius, const glm::vec4 &color, float thickness = 0.05f);
		void FillCircle(const glm::vec3 &p0, float radius, const glm::vec4 &color, float thickness = 0.05f);

		void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color = glm::vec4(1.0f), const bool onTop = false);

		void DrawTransform(const glm::mat4 &transform, float scale = 1.0f, const bool onTop = true);

		float GetLineWidth();
		void  SetLineWidth(float lineWidth);

		// Stats
		struct DrawStatistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4 + LineCount * 2; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6 + LineCount * 2; }
		};

		struct MemoryStatistics
		{
			uint64_t Used           = 0;
			uint64_t TotalAllocated = 0;

			uint64_t GetAllocatedPerFrame() const;
		};

		void             ResetStats();
		DrawStatistics   GetDrawStats();
		MemoryStatistics GetMemoryStats();

		const Renderer2DSpecInfo &GetSpecification() const { return m_Specification; }

	private:
		void Flush();

		void AddQuadBuffer();
		void AddLineBuffer(const bool onTop);
		void AddTextBuffer();
		void AddCircleBuffer();

	private:
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
			glm::vec2 TexCoord;
			float     TexIndex;
			float     TilingFactor;
		};

		struct TextVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
			glm::vec2 TexCoord;
			float     TexIndex;
		};

		struct LineVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
		};

		struct CircleVertex
		{
			glm::vec3 WorldPosition;
			float     Thickness;
			glm::vec2 LocalPosition;
			glm::vec4 Color;
		};

		QuadVertex *&  GetWriteableQuadBuffer();
		LineVertex *&  GetWriteableLineBuffer(const bool onTop);
		CircleVertex *&GetWriteableCircleBuffer();

		static const uint32_t MaxTextureSlots = 32;

		const uint32_t c_MaxVertices;
		const uint32_t c_MaxIndices;

		const uint32_t c_MaxLineVertices;
		const uint32_t c_MaxLineIndices;

		Renderer2DSpecInfo m_Specification;
		RefPtr<CommandBuffer>   m_RenderCommandBuffer;

		RefPtr<Texture2D> m_WhiteTexture;

		using VertexBufferPerFrame = std::vector<RefPtr<VertexBuffer> >;

		// Quads
		RefPtr<RenderPass>                m_QuadPass;
		std::vector<VertexBufferPerFrame> m_QuadVertexBuffers;
		RefPtr<IndexBuffer>               m_QuadIndexBuffer;
		RefPtr<Material>                  m_QuadMaterial;

		uint32_t m_QuadIndexCount    = 0;
		using QuadVertexBasePerFrame = std::vector<QuadVertex *>;
		std::vector<QuadVertexBasePerFrame> m_QuadVertexBufferBases;
		std::vector<QuadVertex *>           m_QuadVertexBufferPtr;
		uint32_t                            m_QuadBufferWriteIndex = 0;

		RefPtr<Pipeline>                  m_CirclePipeline;
		RefPtr<Material>                  m_CircleMaterial;
		std::vector<VertexBufferPerFrame> m_CircleVertexBuffers;
		uint32_t                          m_CircleIndexCount = 0;
		using CircleVertexBasePerFrame                       = std::vector<CircleVertex *>;
		std::vector<CircleVertexBasePerFrame> m_CircleVertexBufferBases;
		std::vector<CircleVertex *>           m_CircleVertexBufferPtr;
		uint32_t                              m_CircleBufferWriteIndex = 0;

		std::array<RefPtr<Texture2D>, MaxTextureSlots> m_TextureSlots;
		uint32_t                                       m_TextureSlotIndex = 1; // 0 = white texture

		glm::vec4 m_QuadVertexPositions[4];

		// Lines
		RefPtr<RenderPass>                m_LinePass;
		std::vector<VertexBufferPerFrame> m_LineVertexBuffers;
		std::vector<VertexBufferPerFrame> m_LineOnTopVertexBuffers;
		RefPtr<IndexBuffer>               m_LineIndexBuffer;
		RefPtr<IndexBuffer>               m_LineOnTopIndexBuffer;
		RefPtr<Material>                  m_LineMaterial;

		uint32_t m_LineIndexCount      = 0;
		uint32_t m_LineOnTopIndexCount = 0;
		using LineVertexBasePerFrame   = std::vector<LineVertex *>;
		std::vector<LineVertexBasePerFrame> m_LineVertexBufferBases;
		std::vector<LineVertexBasePerFrame> m_LineOnTopVertexBufferBases;
		std::vector<LineVertex *>           m_LineVertexBufferPtr;
		std::vector<LineVertex *>           m_LineOnTopVertexBufferPtr;
		uint32_t                            m_LineBufferWriteIndex      = 0;
		uint32_t                            m_LineOnTopBufferWriteIndex = 0;

		glm::mat4 m_CameraViewProj;
		glm::mat4 m_CameraView;
		bool      m_DepthTest = true;

		float m_LineWidth = 1.0f;

		DrawStatistics   m_DrawStats;
		MemoryStatistics m_MemoryStats;

		RefPtr<UniformBufferSet> m_UBSCamera;

		struct UBCamera
		{
			glm::mat4 ViewProjection;
		};
	};
}
