#include "renderer_2d.hpp"

#include "gpu/pipeline.hpp"
#include "gpu/shader.hpp"
#include "renderer/renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace tst
{
	Renderer2D::Renderer2D(const Renderer2DSpecInfo &specification)
		: m_Specification(specification), c_MaxVertices(specification.MaxQuads * 4), c_MaxIndices(specification.MaxQuads * 6),
		  c_MaxLineVertices(specification.MaxLines * 2), c_MaxLineIndices(specification.MaxLines * 2)
	{
		Init();
	}

	Renderer2D::~Renderer2D()
	{
		Shutdown();
	}

	void Renderer2D::Init()
	{
		m_RenderCommandBuffer = make_reference<CommandBuffer>(0, "Renderer2D");

		m_UBSCamera = make_reference<UniformBufferSet>(sizeof(UBCamera));

		m_MemoryStats.TotalAllocated = 0;

		uint32_t framesInFlight = Renderer::getConfigInfo().maxFramesInFlight;

		// Only create internal framebuffer if NOT targeting swapchain
		RefPtr<Framebuffer> framebuffer;
		if (!m_Specification.SwapChainTarget)
		{
			FramebufferSpecInfo framebufferSpec;
			framebufferSpec.Attachments      = {ImageFormat::RGBA32F, ImageFormat::Depth};
			framebufferSpec.Samples          = 1;
			framebufferSpec.ClearColorOnLoad = false;
			framebufferSpec.ClearColor       = {0.1f, 0.5f, 0.5f, 1.0f};
			framebufferSpec.DebugName        = "Renderer2D Framebuffer";

			framebuffer = make_reference<Framebuffer>(framebufferSpec);
		}

		{
			PipelineSpecInfo pipelineSpecification;
			pipelineSpecification.debugName         = "Renderer2D-Quad";
			pipelineSpecification.shader            = Renderer::getShaderLibrary()->get("Renderer2D");
			pipelineSpecification.targetFramebuffer = framebuffer; // Can be null for swapchain target
			pipelineSpecification.backfaceCulling   = false;
			pipelineSpecification.layout            = {
				{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float4, "a_Color"}, {ShaderDataType::Float2, "a_TexCoord"},
				{ShaderDataType::Float, "a_TexIndex"}, {ShaderDataType::Float, "a_TilingFactor"}
			};

			RenderPassSpecInfo quadSpec;
			quadSpec.debugName = "Renderer2D-Quad";
			quadSpec.pipeline  = make_reference<Pipeline>(pipelineSpecification);
			m_QuadPass         = make_reference<RenderPass>(quadSpec);
			m_QuadPass->setInput("Camera", m_UBSCamera);
			TST_ASSERT(m_QuadPass->validate());
			m_QuadPass->bake();

			m_QuadVertexBuffers.resize(1);
			m_QuadVertexBufferBases.resize(1);
			m_QuadVertexBufferPtr.resize(1);

			m_QuadVertexBuffers[0].resize(framesInFlight);
			m_QuadVertexBufferBases[0].resize(framesInFlight);
			for (uint32_t i = 0; i < framesInFlight; i++)
			{
				uint64_t allocationSize   = c_MaxVertices * sizeof(QuadVertex);
				m_QuadVertexBuffers[0][i] = make_reference<VertexBuffer>(allocationSize);
				m_MemoryStats.TotalAllocated += allocationSize;
				m_QuadVertexBufferBases[0][i] = tnew QuadVertex [c_MaxVertices];
			}

			uint32_t *quadIndices = tnew uint32_t[c_MaxIndices];

			uint32_t offset = 0;
			for (uint32_t i = 0; i < c_MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}

			{
				uint64_t allocationSize = c_MaxIndices * sizeof(uint32_t);
				m_QuadIndexBuffer       = make_reference<IndexBuffer>(Buffer(quadIndices, allocationSize));
				m_MemoryStats.TotalAllocated += allocationSize;
			}
			tdelete[] quadIndices;
		}

		m_WhiteTexture = Renderer::getWhiteTexture();

		// Set all texture slots to 0
		m_TextureSlots[0] = m_WhiteTexture;

		m_QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		m_QuadVertexPositions[1] = {-0.5f, 0.5f, 0.0f, 1.0f};
		m_QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
		m_QuadVertexPositions[3] = {0.5f, -0.5f, 0.0f, 1.0f};

		// Lines
		{
			PipelineSpecInfo pipelineSpecification;
			pipelineSpecification.debugName         = "Renderer2D-Line";
			pipelineSpecification.shader            = Renderer::getShaderLibrary()->get("Renderer2D_Line");
			pipelineSpecification.targetFramebuffer = framebuffer;
			pipelineSpecification.topology          = EPrimitiveTopology::eLines;
			pipelineSpecification.lineWidth         = 2.0f;
			pipelineSpecification.layout            = {{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float4, "a_Color"}};

			{
				RenderPassSpecInfo lineSpec;
				lineSpec.debugName = "Renderer2D-Line";
				lineSpec.pipeline  = make_reference<Pipeline>(pipelineSpecification);
				m_LinePass         = make_reference<RenderPass>(lineSpec);
				m_LinePass->setInput("Camera", m_UBSCamera);
				TST_ASSERT(m_LinePass->validate());
				m_LinePass->bake();
			}

			m_LineVertexBuffers.resize(1);
			m_LineOnTopVertexBuffers.resize(1);
			m_LineVertexBufferBases.resize(1);
			m_LineOnTopVertexBufferBases.resize(1);
			m_LineVertexBufferPtr.resize(1);
			m_LineOnTopVertexBufferPtr.resize(1);

			m_LineVertexBuffers[0].resize(framesInFlight);
			m_LineOnTopVertexBuffers[0].resize(framesInFlight);
			m_LineVertexBufferBases[0].resize(framesInFlight);
			m_LineOnTopVertexBufferBases[0].resize(framesInFlight);
			for (uint32_t i = 0; i < framesInFlight; i++)
			{
				uint64_t allocationSize        = c_MaxLineVertices * sizeof(LineVertex);
				m_LineVertexBuffers[0][i]      = make_reference<VertexBuffer>(allocationSize);
				m_LineOnTopVertexBuffers[0][i] = make_reference<VertexBuffer>(allocationSize);
				m_MemoryStats.TotalAllocated += allocationSize + allocationSize;
				m_LineVertexBufferBases[0][i]      = tnew LineVertex [c_MaxLineVertices];
				m_LineOnTopVertexBufferBases[0][i] = tnew LineVertex [c_MaxLineVertices];
			}

			uint32_t *lineIndices = tnew uint32_t[c_MaxLineIndices];
			for (uint32_t i    = 0; i < c_MaxLineIndices; i++)
				lineIndices[i] = i;

			{
				uint64_t allocationSize = c_MaxLineIndices * sizeof(uint32_t);
				m_LineIndexBuffer       = make_reference<IndexBuffer>(Buffer(lineIndices, allocationSize));
				m_LineOnTopIndexBuffer  = make_reference<IndexBuffer>(Buffer(lineIndices, allocationSize));
				m_MemoryStats.TotalAllocated += allocationSize + allocationSize;
			}
			tdelete[] lineIndices;
		}
		// Circles
		{
			PipelineSpecInfo pipelineSpecification;
			pipelineSpecification.debugName         = "Renderer2D-Circle";
			pipelineSpecification.shader            = Renderer::getShaderLibrary()->get("Renderer2D_Circle");
			pipelineSpecification.backfaceCulling   = false;
			pipelineSpecification.targetFramebuffer = framebuffer;
			pipelineSpecification.layout            = {
				{ShaderDataType::Float3, "a_WorldPosition"}, {ShaderDataType::Float, "a_Thickness"}, {ShaderDataType::Float2, "a_LocalPosition"},
				{ShaderDataType::Float4, "a_Color"}
			};
			m_CirclePipeline = make_reference<Pipeline>(pipelineSpecification);
			m_CircleMaterial = make_reference<Material>(pipelineSpecification.shader);

			m_CircleVertexBuffers.resize(1);
			m_CircleVertexBufferBases.resize(1);
			m_CircleVertexBufferPtr.resize(1);

			m_CircleVertexBuffers[0].resize(framesInFlight);
			m_CircleVertexBufferBases[0].resize(framesInFlight);
			for (uint32_t i = 0; i < framesInFlight; i++)
			{
				uint64_t allocationSize     = c_MaxVertices * sizeof(QuadVertex);
				m_CircleVertexBuffers[0][i] = make_reference<VertexBuffer>(allocationSize);
				m_MemoryStats.TotalAllocated += allocationSize;
				m_CircleVertexBufferBases[0][i] = tnew CircleVertex [c_MaxVertices];
			}
		}

		m_QuadMaterial = make_reference<Material>(m_QuadPass->getPipeline()->getShader(), "QuadMaterial");
		m_LineMaterial = make_reference<Material>(m_LinePass->getPipeline()->getShader(), "LineMaterial");
	}

	void Renderer2D::Shutdown()
	{
		for (auto buffers: m_QuadVertexBufferBases)
		{
			for (auto buffer: buffers)
				tdelete[] buffer;
		}

		for (auto buffers: m_LineVertexBufferBases)
		{
			for (auto buffer: buffers)
				tdelete[] buffer;
		}

		for (auto buffers: m_CircleVertexBufferBases)
		{
			for (auto buffer: buffers)
				tdelete[] buffer;
		}
	}

	void Renderer2D::BeginScene(const glm::mat4 &viewProj, const glm::mat4 &view, bool depthTest)
	{
		uint32_t frameIndex = Renderer::getCurrentFrameIndex();

		const bool updatedAnyShaders = Renderer::updateDirtyShaders();
		if (updatedAnyShaders)
		{
			// Update materials that aren't set on use.
		}
		m_CameraViewProj = viewProj;
		m_CameraView     = view;
		m_DepthTest      = depthTest;

		Renderer::submit([ubsCamera = m_UBSCamera, viewProj]() mutable
		{
			uint32_t bufferIndex = Renderer::_getCurrentFrameIndex();
			ubsCamera->RT_Get()->RT_SetData(&viewProj, sizeof(UBCamera));
		});

		m_QuadIndexCount = 0;
		for (uint32_t i              = 0; i < m_QuadVertexBufferPtr.size(); i++)
			m_QuadVertexBufferPtr[i] = m_QuadVertexBufferBases[i][frameIndex];

		m_LineIndexCount = 0;
		for (uint32_t i              = 0; i < m_LineVertexBufferPtr.size(); i++)
			m_LineVertexBufferPtr[i] = m_LineVertexBufferBases[i][frameIndex];

		m_LineOnTopIndexCount = 0;
		for (uint32_t i                   = 0; i < m_LineOnTopVertexBufferPtr.size(); i++)
			m_LineOnTopVertexBufferPtr[i] = m_LineOnTopVertexBufferBases[i][frameIndex];

		m_CircleIndexCount = 0;
		for (uint32_t i                = 0; i < m_CircleVertexBufferPtr.size(); i++)
			m_CircleVertexBufferPtr[i] = m_CircleVertexBufferBases[i][frameIndex];

		for (uint32_t i       = 1; i < m_TextureSlots.size(); i++)
			m_TextureSlots[i] = nullptr;
	}

	void Renderer2D::EndScene()
	{
		uint32_t frameIndex = Renderer::getCurrentFrameIndex();

		m_RenderCommandBuffer->beginRecording();

		uint32_t dataSize = 0;

		// Quads
		for (uint32_t i = 0; i <= m_QuadBufferWriteIndex; i++)
		{
			dataSize = (uint32_t) ((uint8_t *) m_QuadVertexBufferPtr[i] - (uint8_t *) m_QuadVertexBufferBases[i][frameIndex]);
			if (dataSize)
			{
				uint32_t indexCount = i == m_QuadBufferWriteIndex ? m_QuadIndexCount - (c_MaxIndices * i) : c_MaxIndices;
				m_QuadVertexBuffers[i][frameIndex]->setData(m_QuadVertexBufferBases[i][frameIndex], dataSize, 0);

				for (uint32_t i = 0; i < m_TextureSlots.size(); i++)
				{
					if (m_TextureSlots[i])
						m_QuadMaterial->set("u_Textures", m_TextureSlots[i], i);
					else
						m_QuadMaterial->set("u_Textures", m_WhiteTexture, i);
				}

				Renderer::beginRenderPass(m_RenderCommandBuffer, m_QuadPass);
				Renderer::renderGeometry(m_RenderCommandBuffer, m_QuadPass->getPipeline(), m_QuadMaterial, m_QuadVertexBuffers[i][frameIndex], m_QuadIndexBuffer, {1.0f},
										 indexCount);
				Renderer::endRenderPass(m_RenderCommandBuffer);

				m_DrawStats.DrawCalls++;
				m_MemoryStats.Used += dataSize;
			}
		}

		// Lines
		m_LinePass->getPipeline()->getSpecInfo().depthTest = true;
		for (uint32_t i = 0; i <= m_LineBufferWriteIndex; i++)
		{
			dataSize = (uint32_t) ((uint8_t *) m_LineVertexBufferPtr[i] - (uint8_t *) m_LineVertexBufferBases[i][frameIndex]);
			if (dataSize)
			{
				uint32_t indexCount = i == m_LineBufferWriteIndex ? m_LineIndexCount - (c_MaxLineIndices * i) : c_MaxLineIndices;
				m_LineVertexBuffers[i][frameIndex]->setData(m_LineVertexBufferBases[i][frameIndex], dataSize, 0);

				Renderer::beginRenderPass(m_RenderCommandBuffer, m_LinePass);
				Renderer::renderGeometry(m_RenderCommandBuffer, m_LinePass->getSpecInfo().pipeline, m_LineMaterial, m_LineVertexBuffers[i][frameIndex], m_LineIndexBuffer,
										 {1.0f}, indexCount);
				Renderer::endRenderPass(m_RenderCommandBuffer);

				m_DrawStats.DrawCalls++;
				m_MemoryStats.Used += dataSize;
			}
		}

		m_LinePass->getPipeline()->getSpecInfo().depthTest = false;
		for (uint32_t i = 0; i <= m_LineOnTopBufferWriteIndex; i++)
		{
			dataSize = (uint32_t) ((uint8_t *) m_LineOnTopVertexBufferPtr[i] - (uint8_t *) m_LineOnTopVertexBufferBases[i][frameIndex]);
			if (dataSize)
			{
				uint32_t indexCount = i == m_LineOnTopBufferWriteIndex ? m_LineOnTopIndexCount - (c_MaxLineIndices * i) : c_MaxLineIndices;
				m_LineOnTopVertexBuffers[i][frameIndex]->setData(m_LineOnTopVertexBufferBases[i][frameIndex], dataSize, 0);

				Renderer::beginRenderPass(m_RenderCommandBuffer, m_LinePass);
				Renderer::renderGeometry(m_RenderCommandBuffer, m_LinePass->getSpecInfo().pipeline, m_LineMaterial, m_LineOnTopVertexBuffers[i][frameIndex],
										 m_LineOnTopIndexBuffer, {1.0f}, indexCount);
				Renderer::endRenderPass(m_RenderCommandBuffer);

				m_DrawStats.DrawCalls++;
				m_MemoryStats.Used += dataSize;
			}
		}

		m_RenderCommandBuffer->endRecording();
		m_RenderCommandBuffer->submit();
	}

	void Renderer2D::Flush()
	{
	}

	RefPtr<RenderPass> Renderer2D::GetTargetRenderPass()
	{
		return m_QuadPass;
	}

	void Renderer2D::SetTargetFramebuffer(RefPtr<Framebuffer> framebuffer)
	{
		if (!framebuffer)
			return;

		// Update quad pipeline
		{
			PipelineSpecInfo pipelineSpec      = m_QuadPass->getSpecInfo().pipeline->getSpecInfo();
			pipelineSpec.targetFramebuffer     = framebuffer;
			RenderPassSpecInfo &renderpassSpec = m_QuadPass->getSpecInfo();
			renderpassSpec.pipeline            = make_reference<Pipeline>(pipelineSpec);

			// Re-initialize the render pass with new pipeline
			m_QuadPass = make_reference<RenderPass>(renderpassSpec);
			m_QuadPass->setInput("Camera", m_UBSCamera);
			m_QuadPass->bake();
		}

		// Update line pipeline
		{
			PipelineSpecInfo pipelineSpec      = m_LinePass->getSpecInfo().pipeline->getSpecInfo();
			pipelineSpec.targetFramebuffer     = framebuffer;
			RenderPassSpecInfo &renderpassSpec = m_LinePass->getSpecInfo();
			renderpassSpec.pipeline            = make_reference<Pipeline>(pipelineSpec);

			// Re-initialize the render pass with new pipeline
			m_LinePass = make_reference<RenderPass>(renderpassSpec);
			m_LinePass->setInput("Camera", m_UBSCamera);
			m_LinePass->bake();
		}

		// Update circle pipeline
		{
			PipelineSpecInfo pipelineSpec  = m_CirclePipeline->getSpecInfo();
			pipelineSpec.targetFramebuffer = framebuffer;
			m_CirclePipeline               = make_reference<Pipeline>(pipelineSpec);
		}
	}

	void Renderer2D::OnRecreateSwapchain()
	{
	}

	void Renderer2D::AddQuadBuffer()
	{
		uint32_t framesInFlight = Renderer::getConfigInfo().maxFramesInFlight;

		VertexBufferPerFrame &  newVertexBuffer     = m_QuadVertexBuffers.emplace_back();
		QuadVertexBasePerFrame &newVertexBufferBase = m_QuadVertexBufferBases.emplace_back();

		newVertexBuffer.resize(framesInFlight);
		newVertexBufferBase.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			uint64_t allocationSize = c_MaxVertices * sizeof(QuadVertex);
			newVertexBuffer[i]      = make_reference<VertexBuffer>(allocationSize);
			m_MemoryStats.TotalAllocated += allocationSize;
			newVertexBufferBase[i] = tnew QuadVertex [c_MaxVertices];
		}
	}

	void Renderer2D::AddLineBuffer(const bool onTop)
	{
		uint32_t framesInFlight = Renderer::getConfigInfo().maxFramesInFlight;

		VertexBufferPerFrame &  newVertexBuffer     = onTop ? m_LineOnTopVertexBuffers.emplace_back() : m_LineVertexBuffers.emplace_back();
		LineVertexBasePerFrame &newVertexBufferBase = onTop ? m_LineOnTopVertexBufferBases.emplace_back() : m_LineVertexBufferBases.emplace_back();

		newVertexBuffer.resize(framesInFlight);
		newVertexBufferBase.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			uint64_t allocationSize = c_MaxLineVertices * sizeof(LineVertex);
			newVertexBuffer[i]      = make_reference<VertexBuffer>(allocationSize);
			m_MemoryStats.TotalAllocated += allocationSize;
			newVertexBufferBase[i] = tnew LineVertex [c_MaxLineVertices];
		}
	}

	void Renderer2D::AddCircleBuffer()
	{
		uint32_t framesInFlight = Renderer::getConfigInfo().maxFramesInFlight;

		VertexBufferPerFrame &    newVertexBuffer     = m_CircleVertexBuffers.emplace_back();
		CircleVertexBasePerFrame &newVertexBufferBase = m_CircleVertexBufferBases.emplace_back();

		newVertexBuffer.resize(framesInFlight);
		newVertexBufferBase.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			uint64_t allocationSize = c_MaxVertices * sizeof(CircleVertex);
			newVertexBuffer[i]      = make_reference<VertexBuffer>(allocationSize);
			m_MemoryStats.TotalAllocated += allocationSize;
			newVertexBufferBase[i] = tnew CircleVertex [c_MaxVertices];
		}
	}

	Renderer2D::QuadVertex *&Renderer2D::GetWriteableQuadBuffer()
	{
		uint32_t frameIndex = Renderer::getCurrentFrameIndex();

		m_QuadBufferWriteIndex = m_QuadIndexCount / c_MaxIndices;
		if (m_QuadBufferWriteIndex >= m_QuadVertexBufferBases.size())
		{
			AddQuadBuffer();
			m_QuadVertexBufferPtr.emplace_back();
			m_QuadVertexBufferPtr[m_QuadBufferWriteIndex] = m_QuadVertexBufferBases[m_QuadBufferWriteIndex][frameIndex];
		}

		return m_QuadVertexBufferPtr[m_QuadBufferWriteIndex];
	}

	Renderer2D::LineVertex *&Renderer2D::GetWriteableLineBuffer(const bool onTop)
	{
		uint32_t frameIndex = Renderer::getCurrentFrameIndex();

		if (onTop)
		{
			m_LineOnTopBufferWriteIndex = m_LineOnTopIndexCount / c_MaxLineIndices;
			if (m_LineOnTopBufferWriteIndex >= m_LineOnTopVertexBufferBases.size())
			{
				AddLineBuffer(onTop);
				m_LineOnTopVertexBufferPtr.emplace_back();
				m_LineOnTopVertexBufferPtr[m_LineOnTopBufferWriteIndex] = m_LineOnTopVertexBufferBases[m_LineOnTopBufferWriteIndex][frameIndex];
			}

			return m_LineOnTopVertexBufferPtr[m_LineOnTopBufferWriteIndex];
		}
		else
		{
			m_LineBufferWriteIndex = m_LineIndexCount / c_MaxLineIndices;
			if (m_LineBufferWriteIndex >= m_LineVertexBufferBases.size())
			{
				AddLineBuffer(onTop);
				m_LineVertexBufferPtr.emplace_back();
				m_LineVertexBufferPtr[m_LineBufferWriteIndex] = m_LineVertexBufferBases[m_LineBufferWriteIndex][frameIndex];
			}

			return m_LineVertexBufferPtr[m_LineBufferWriteIndex];
		}
	}

	Renderer2D::CircleVertex *&Renderer2D::GetWriteableCircleBuffer()
	{
		uint32_t frameIndex = Renderer::getCurrentFrameIndex();

		m_CircleBufferWriteIndex = m_CircleIndexCount / c_MaxIndices;
		if (m_CircleBufferWriteIndex >= m_CircleVertexBufferBases.size())
		{
			AddCircleBuffer();
			m_CircleVertexBufferPtr.emplace_back();
			m_CircleVertexBufferPtr[m_CircleBufferWriteIndex] = m_CircleVertexBufferBases[m_CircleBufferWriteIndex][frameIndex];
		}

		return m_CircleVertexBufferPtr[m_CircleBufferWriteIndex];
	}

	void Renderer2D::DrawQuad(const glm::mat4 &transform, const glm::vec4 &color)
	{
		uint32_t frameIndex = Renderer::getCurrentFrameIndex();

		constexpr size_t    quadVertexCount = 4;
		const float         textureIndex    = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
		const float         tilingFactor    = 1.0f;

		m_QuadBufferWriteIndex = m_QuadIndexCount / c_MaxIndices;
		if (m_QuadBufferWriteIndex >= m_QuadVertexBufferBases.size())
		{
			AddQuadBuffer();
			m_QuadVertexBufferPtr.emplace_back();
			m_QuadVertexBufferPtr[m_QuadBufferWriteIndex] = m_QuadVertexBufferBases[m_QuadBufferWriteIndex][frameIndex];
		}

		auto &bufferPtr = m_QuadVertexBufferPtr[m_QuadBufferWriteIndex];
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			bufferPtr->Position     = transform * m_QuadVertexPositions[i];
			bufferPtr->Color        = color;
			bufferPtr->TexCoord     = textureCoords[i];
			bufferPtr->TexIndex     = textureIndex;
			bufferPtr->TilingFactor = tilingFactor;
			bufferPtr++;
		}

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4 &transform, const RefPtr<Texture2D> &texture, float tilingFactor, const glm::vec4 &tintColor, glm::vec2 uv0, glm::vec2 uv1)
	{
		constexpr size_t quadVertexCount = 4;
		glm::vec2        textureCoords[] = {uv0, {uv1.x, uv0.y}, uv1, {uv0.x, uv1.y}};

		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i]->GetHash() == texture->GetHash())
			{
				textureIndex = (float) i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			//if (m_TextureSlotIndex >= MaxTextureSlots)
			//	FlushAndReset();

			textureIndex                       = (float) m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		auto &bufferPtr = m_QuadVertexBufferPtr[m_QuadBufferWriteIndex];
		for (size_t i = 0; i < quadVertexCount; i++)
		{
			bufferPtr->Position     = transform * m_QuadVertexPositions[i];
			bufferPtr->Color        = tintColor;
			bufferPtr->TexCoord     = textureCoords[i];
			bufferPtr->TexIndex     = textureIndex;
			bufferPtr->TilingFactor = tilingFactor;
			bufferPtr++;
		}

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
	{
		DrawQuad({position.x, position.y, 0.0f}, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
	{
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		auto &bufferPtr         = GetWriteableQuadBuffer();
		bufferPtr->Position     = transform * m_QuadVertexPositions[0];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {0.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[1];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {1.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[2];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {1.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[3];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {0.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::vec2 &position, const glm::vec2 &size, const RefPtr<Texture2D> &texture, float tilingFactor, const glm::vec4 &tintColor,
							  glm::vec2        uv0, glm::vec2             uv1)
	{
		DrawQuad({position.x, position.y, 0.0f}, size, texture, tilingFactor, tintColor, uv0, uv1);
	}

	void Renderer2D::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const RefPtr<Texture2D> &texture, float tilingFactor, const glm::vec4 &tintColor,
							  glm::vec2        uv0, glm::vec2             uv1)
	{
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i].get() == texture.get())
			{
				textureIndex = (float) i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex                       = (float) m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		glm::vec2 textureCoords[] = {uv0, {uv1.x, uv0.y}, uv1, {uv0.x, uv1.y}};

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		auto &bufferPtr         = GetWriteableQuadBuffer();
		bufferPtr->Position     = transform * m_QuadVertexPositions[0];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = textureCoords[0];
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[1];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = textureCoords[1];
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[2];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = textureCoords[2];
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[3];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = textureCoords[3];
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
	{
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		glm::vec3 camRightWS = {m_CameraView[0][0], m_CameraView[1][0], m_CameraView[2][0]};
		glm::vec3 camUpWS    = {m_CameraView[0][1], m_CameraView[1][1], m_CameraView[2][1]};

		auto &bufferPtr         = GetWriteableQuadBuffer();
		bufferPtr->Position     = position + camRightWS * (m_QuadVertexPositions[0].x) * size.x + camUpWS * m_QuadVertexPositions[0].y * size.y;
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {0.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = position + camRightWS * m_QuadVertexPositions[1].x * size.x + camUpWS * m_QuadVertexPositions[1].y * size.y;
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {1.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = position + camRightWS * m_QuadVertexPositions[2].x * size.x + camUpWS * m_QuadVertexPositions[2].y * size.y;
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {1.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = position + camRightWS * m_QuadVertexPositions[3].x * size.x + camUpWS * m_QuadVertexPositions[3].y * size.y;
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {0.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size, const RefPtr<Texture2D> &texture, float tilingFactor, const glm::vec4 &tintColor)
	{
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i]->GetHash() == texture->GetHash())
			{
				textureIndex = (float) i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex                       = (float) m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		glm::vec3 camRightWS = {m_CameraView[0][0], m_CameraView[1][0], m_CameraView[2][0]};
		glm::vec3 camUpWS    = {m_CameraView[0][1], m_CameraView[1][1], m_CameraView[2][1]};

		auto &bufferPtr         = GetWriteableQuadBuffer();
		bufferPtr->Position     = position + camRightWS * (m_QuadVertexPositions[0].x) * size.x + camUpWS * m_QuadVertexPositions[0].y * size.y;
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {0.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = position + camRightWS * m_QuadVertexPositions[1].x * size.x + camUpWS * m_QuadVertexPositions[1].y * size.y;
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {0.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = position + camRightWS * m_QuadVertexPositions[2].x * size.x + camUpWS * m_QuadVertexPositions[2].y * size.y;
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {1.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = position + camRightWS * m_QuadVertexPositions[3].x * size.x + camUpWS * m_QuadVertexPositions[3].y * size.y;
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {1.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color)
	{
		DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color)
	{
		const float textureIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
							  glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		auto &bufferPtr         = GetWriteableQuadBuffer();
		bufferPtr->Position     = transform * m_QuadVertexPositions[0];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {0.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[1];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {1.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[2];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {1.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[3];
		bufferPtr->Color        = color;
		bufferPtr->TexCoord     = {0.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float rotation, const RefPtr<Texture2D> &texture, float tilingFactor,
									 const glm::vec4 &tintColor)
	{
		DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float rotation, const RefPtr<Texture2D> &texture, float tilingFactor,
									 const glm::vec4 &tintColor)
	{
		float textureIndex = 0.0f;
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i]->GetHash() == texture->GetHash())
			{
				textureIndex = (float) i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex                       = (float) m_TextureSlotIndex;
			m_TextureSlots[m_TextureSlotIndex] = texture;
			m_TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
							  glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		auto &bufferPtr         = GetWriteableQuadBuffer();
		bufferPtr->Position     = transform * m_QuadVertexPositions[0];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {0.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[1];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {1.0f, 0.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[2];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {1.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		bufferPtr->Position     = transform * m_QuadVertexPositions[3];
		bufferPtr->Color        = tintColor;
		bufferPtr->TexCoord     = {0.0f, 1.0f};
		bufferPtr->TexIndex     = textureIndex;
		bufferPtr->TilingFactor = tilingFactor;
		bufferPtr++;

		m_QuadIndexCount += 6;

		m_DrawStats.QuadCount++;
	}

	void Renderer2D::DrawRotatedRect(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color, const bool onTop)
	{
		DrawRotatedRect({position.x, position.y, 0.0f}, size, rotation, color, onTop);
	}

	void Renderer2D::DrawRotatedRect(const glm::vec3 &position, const glm::vec2 &size, float rotation, const glm::vec4 &color, const bool onTop)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation, {0.0f, 0.0f, 1.0f}) *
							  glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

		glm::vec3 positions[4] = {
			transform * m_QuadVertexPositions[0], transform * m_QuadVertexPositions[1], transform * m_QuadVertexPositions[2], transform * m_QuadVertexPositions[3]
		};

		for (int i = 0; i < 4; i++)
		{
			auto &v0 = positions[i];
			auto &v1 = positions[(i + 1) % 4];

			auto &bufferPtr     = GetWriteableLineBuffer(onTop);
			bufferPtr->Position = v0;
			bufferPtr->Color    = color;
			bufferPtr++;

			bufferPtr->Position = v1;
			bufferPtr->Color    = color;
			bufferPtr++;

			m_LineIndexCount += 2;
			m_DrawStats.LineCount++;
		}
	}

	void Renderer2D::FillCircle(const glm::vec2 &position, float radius, const glm::vec4 &color, const float thickness)
	{
		FillCircle({position.x, position.y, 0.0f}, radius, color, thickness);
	}

	void Renderer2D::FillCircle(const glm::vec3 &position, float radius, const glm::vec4 &color, const float thickness)
	{
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {radius * 2.0f, radius * 2.0f, 1.0f});

		auto &bufferPtr = GetWriteableCircleBuffer();
		for (int i = 0; i < 4; i++)
		{
			bufferPtr->WorldPosition = transform * m_QuadVertexPositions[i];
			bufferPtr->Thickness     = thickness;
			bufferPtr->LocalPosition = m_QuadVertexPositions[i] * 2.0f;
			bufferPtr->Color         = color;
			bufferPtr++;

			m_CircleIndexCount += 6;
			m_DrawStats.QuadCount++;
		}
	}

	void Renderer2D::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color, const bool onTop)
	{
		auto &bufferPtr     = GetWriteableLineBuffer(onTop);
		bufferPtr->Position = p0;
		bufferPtr->Color    = color;
		bufferPtr++;

		bufferPtr->Position = p1;
		bufferPtr->Color    = color;
		bufferPtr++;

		if (onTop)
			m_LineOnTopIndexCount += 2;
		else
			m_LineIndexCount += 2;

		m_DrawStats.LineCount++;
	}

	void Renderer2D::DrawTransform(const glm::mat4 &transform, float scale /*= 1.0f*/, const bool onTop)
	{
		glm::vec3 p0 = transform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::vec3 p1 = transform * glm::vec4(scale, 0.0f, 0.0f, 1.0f);
		DrawLine(p0, p1, {1.0f, 0.0f, 0.0f, 1.0f}, onTop);

		p1 = transform * glm::vec4(0.0f, scale, 0.0f, 1.0f);
		DrawLine(p0, p1, {0.0f, 1.0f, 0.0f, 1.0f}, onTop);

		p1 = transform * glm::vec4(0.0f, 0.0f, scale, 1.0f);
		DrawLine(p0, p1, {0.0f, 0.0f, 1.0f, 1.0f}, onTop);
	}

	void Renderer2D::DrawCircle(const glm::vec3 &position, const glm::vec3 &rotation, float radius, const glm::vec4 &color, const bool onTop)
	{
		const glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), rotation.x, {1.0f, 0.0f, 0.0f}) *
									glm::rotate(glm::mat4(1.0f), rotation.y, {0.0f, 1.0f, 0.0f}) * glm::rotate(glm::mat4(1.0f), rotation.z, {0.0f, 0.0f, 1.0f}) *
									glm::scale(glm::mat4(1.0f), glm::vec3(radius));

		DrawCircle(transform, color, onTop);
	}

	void Renderer2D::DrawCircle(const glm::mat4 &transform, const glm::vec4 &color, const bool onTop)
	{
		int segments = 32;
		for (int i = 0; i < segments; i++)
		{
			float     angle         = 2.0f * glm::pi<float>() * (float) i / segments;
			glm::vec4 startPosition = {glm::cos(angle), glm::sin(angle), 0.0f, 1.0f};
			angle                   = 2.0f * glm::pi<float>() * (float) ((i + 1) % segments) / segments;
			glm::vec4 endPosition   = {glm::cos(angle), glm::sin(angle), 0.0f, 1.0f};

			glm::vec3 p0 = transform * startPosition;
			glm::vec3 p1 = transform * endPosition;
			DrawLine(p0, p1, color, onTop);
		}
	}

	static bool NextLine(int index, const std::vector<int> &lines)
	{
		for (int line: lines)
		{
			if (line == index)
				return true;
		}
		return false;
	}

	float Renderer2D::GetLineWidth()
	{
		return m_LineWidth;
	}

	void Renderer2D::SetLineWidth(float lineWidth)
	{
		m_LineWidth = lineWidth;

		if (m_LinePass)
			m_LinePass->getPipeline()->getSpecInfo().lineWidth = lineWidth;
	}

	void Renderer2D::ResetStats()
	{
		memset(&m_DrawStats, 0, sizeof(DrawStatistics));
		m_MemoryStats.Used = 0;
	}

	Renderer2D::DrawStatistics Renderer2D::GetDrawStats()
	{
		return m_DrawStats;
	}

	Renderer2D::MemoryStatistics Renderer2D::GetMemoryStats()
	{
		return m_MemoryStats;
	}

	uint64_t Renderer2D::MemoryStatistics::GetAllocatedPerFrame() const
	{
		return TotalAllocated / Renderer::getConfigInfo().maxFramesInFlight;
	}
}
