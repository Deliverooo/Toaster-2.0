#include "imgui_renderer.hpp"

#include "gpu/shader.hpp"
#include "nvrhi/utils.h"
#include "platform/application.hpp"
#include "renderer/renderer.hpp"

namespace tst
{
	struct VERTEX_CONSTANT_BUFFER
	{
		float mvp[4][4];
	};

	bool ImGuiRenderer::updateFontTexture()
	{
		nvrhi::IDevice *device = Application::getGraphicsDevice();

		ImGuiIO &io = ImGui::GetIO();

		io.BackendRendererName = "ToasterImGuiRenderer";
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
		io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports on the Renderer side (optional)
		// If the font texture exists and is bound to ImGui, we're done.
		// Note: ImGui_Renderer will reset io.Fonts->TexID when new fonts are added.
		if (m_FontTexture && io.Fonts->TexID)
			return true;

		unsigned char *pixels;
		int            width, height;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		if (!pixels)
			return false;

		nvrhi::TextureDesc textureDesc;
		textureDesc.width     = width;
		textureDesc.height    = height;
		textureDesc.format    = nvrhi::Format::RGBA8_UNORM;
		textureDesc.debugName = "ImGui font texture";

		m_FontTexture = device->createTexture(textureDesc);

		if (m_FontTexture == nullptr)
			return false;

		m_RenderCommandBuffer->_beginRecording();
		nvrhi::CommandListHandle commandList = m_RenderCommandBuffer->getActiveCommandBuffer();

		commandList->beginTrackingTextureState(m_FontTexture, nvrhi::AllSubresources, nvrhi::ResourceStates::Common);

		commandList->writeTexture(m_FontTexture, 0, 0, pixels, width * 4);

		commandList->setPermanentTextureState(m_FontTexture, nvrhi::ResourceStates::ShaderResource);
		commandList->commitBarriers();

		m_RenderCommandBuffer->_endRecording();
		m_RenderCommandBuffer->_submit();

		io.Fonts->TexID = (ImTextureID) m_FontTexture.Get();

		return true;
	}

	bool ImGuiRenderer::init()
	{
		auto device = Application::getGraphicsDevice();

		m_RenderCommandBuffer = make_reference<CommandBuffer>(0, "ImGuiRenderer");

		RefPtr<Shader> imguiShader = Renderer::getShaderLibrary()->get("ImGui");
		m_VertexShader             = imguiShader->getHandle(nvrhi::ShaderType::Vertex);
		m_PixelShader              = imguiShader->getHandle(nvrhi::ShaderType::Pixel);

		// create attribute layout object
		nvrhi::VertexAttributeDesc vertexAttribLayout[] = {
			{"POSITION", nvrhi::Format::RG32_FLOAT, 1, 0, offsetof(ImDrawVert, pos), sizeof(ImDrawVert), false},
			{"TEXCOORD", nvrhi::Format::RG32_FLOAT, 1, 0, offsetof(ImDrawVert, uv), sizeof(ImDrawVert), false},
			{"COLOR", nvrhi::Format::RGBA8_UNORM, 1, 0, offsetof(ImDrawVert, col), sizeof(ImDrawVert), false},
		};

		m_ShaderAttribLayout = device->createInputLayout(vertexAttribLayout, std::size(vertexAttribLayout), m_VertexShader);

		// create PSO
		{
			nvrhi::BlendState blendState;
			blendState.targets[0].setBlendEnable(true).setSrcBlend(nvrhi::BlendFactor::SrcAlpha).setDestBlend(nvrhi::BlendFactor::InvSrcAlpha).
					setSrcBlendAlpha(nvrhi::BlendFactor::One).setDestBlendAlpha(nvrhi::BlendFactor::InvSrcAlpha);

			auto rasterState = nvrhi::RasterState().setFillSolid().setCullNone().setScissorEnable(true).setDepthClipEnable(true);

			auto depthStencilState = nvrhi::DepthStencilState().disableDepthTest().enableDepthWrite().disableStencil().setDepthFunc(nvrhi::ComparisonFunc::Always);

			nvrhi::RenderState renderState;
			renderState.blendState        = blendState;
			renderState.depthStencilState = depthStencilState;
			renderState.rasterState       = rasterState;

			nvrhi::BindingLayoutDesc layoutDesc;
			layoutDesc.visibility = nvrhi::ShaderType::All;
			layoutDesc.bindings   = {
				nvrhi::BindingLayoutItem::PushConstants(0, sizeof(tsm::vec2i) * 2), nvrhi::BindingLayoutItem::Texture_SRV(0), nvrhi::BindingLayoutItem::Sampler(1)
			};
			m_BindingLayout = device->createBindingLayout(layoutDesc);

			m_BasePSODesc.primType       = nvrhi::PrimitiveType::TriangleList;
			m_BasePSODesc.inputLayout    = m_ShaderAttribLayout;
			m_BasePSODesc.VS             = m_VertexShader;
			m_BasePSODesc.PS             = m_PixelShader;
			m_BasePSODesc.renderState    = renderState;
			m_BasePSODesc.bindingLayouts = {m_BindingLayout};
		}

		{
			const auto desc = nvrhi::SamplerDesc().setAllAddressModes(nvrhi::SamplerAddressMode::Wrap).setAllFilters(true);

			m_FontSampler = device->createSampler(desc);

			if (m_FontSampler == nullptr)
				return false;
		}

		return true;
	}

	bool ImGuiRenderer::reallocateBuffer(nvrhi::BufferHandle &buffer, size_t requiredSize, size_t reallocateSize, const bool indexBuffer)
	{
		nvrhi::IDevice *device = Application::getGraphicsDevice();

		if (buffer == nullptr || size_t(buffer->getDesc().byteSize) < requiredSize)
		{
			nvrhi::BufferDesc desc;
			desc.byteSize           = uint32_t(reallocateSize);
			desc.structStride       = 0;
			desc.debugName          = indexBuffer ? "ImGui index buffer" : "ImGui vertex buffer";
			desc.canHaveUAVs        = false;
			desc.isVertexBuffer     = !indexBuffer;
			desc.isIndexBuffer      = indexBuffer;
			desc.isDrawIndirectArgs = false;
			desc.isVolatile         = false;
			desc.initialState       = indexBuffer ? nvrhi::ResourceStates::IndexBuffer : nvrhi::ResourceStates::VertexBuffer;
			desc.keepInitialState   = true;

			buffer = device->createBuffer(desc);

			if (!buffer)
			{
				return false;
			}
		}

		return true;
	}

	nvrhi::GraphicsPipelineHandle ImGuiRenderer::getOrCreatePipeline(VulkanSwapChain *swapchain)
	{
		uint32_t currentFramebufferIndex = swapchain->getCurrentBackBufferIndex();
		auto &   swapchainPipelineCache  = m_PipelineCache[swapchain];
		TST_ASSERT(currentFramebufferIndex < swapchainPipelineCache.Pipelines.max_size());

		nvrhi::FramebufferHandle targetFramebuffer = swapchain->getCurrentFramebuffer();

		nvrhi::GraphicsPipelineHandle pipeline   = swapchainPipelineCache.Pipelines[currentFramebufferIndex];
		bool                          invalidate = !pipeline || swapchainPipelineCache.Framebuffers[currentFramebufferIndex] != targetFramebuffer;
		if (invalidate)
		{
			nvrhi::DeviceHandle device                                   = Application::getGraphicsDevice();
			pipeline                                                     = device->createGraphicsPipeline(m_BasePSODesc, targetFramebuffer->getFramebufferInfo());
			swapchainPipelineCache.Pipelines[currentFramebufferIndex]    = pipeline;
			swapchainPipelineCache.Framebuffers[currentFramebufferIndex] = targetFramebuffer;
		}
		return pipeline;
	}

	nvrhi::IBindingSet *ImGuiRenderer::getBindingSet(nvrhi::ITexture *texture)
	{
		nvrhi::IDevice *device = Application::getGraphicsDevice();

		auto iter = m_BindingsCache.find(texture);
		if (iter != m_BindingsCache.end())
		{
			return iter->second;
		}

		nvrhi::BindingSetDesc desc;

		desc.bindings = {
			nvrhi::BindingSetItem::PushConstants(0, sizeof(float) * 2), nvrhi::BindingSetItem::Texture_SRV(0, texture), nvrhi::BindingSetItem::Sampler(1, m_FontSampler)
		};

		nvrhi::BindingSetHandle binding = device->createBindingSet(desc, m_BindingLayout);
		TST_ASSERT(binding);

		m_BindingsCache[texture] = binding;
		return binding;
	}

	bool ImGuiRenderer::updateGeometry(ImDrawData *drawData)
	{
		nvrhi::CommandListHandle commandList = m_RenderCommandBuffer->getActiveCommandBuffer();

		// create/resize vertex and index buffers if needed
		if (!reallocateBuffer(m_VertexBuffer, drawData->TotalVtxCount * sizeof(ImDrawVert), (drawData->TotalVtxCount + 5000) * sizeof(ImDrawVert), false))
		{
			return false;
		}

		if (!reallocateBuffer(m_IndexBuffer, drawData->TotalIdxCount * sizeof(ImDrawIdx), (drawData->TotalIdxCount + 5000) * sizeof(ImDrawIdx), true))
		{
			return false;
		}

		m_VertexBufferData.resize(m_VertexBuffer->getDesc().byteSize / sizeof(ImDrawVert));
		m_IndexBufferData.resize(m_IndexBuffer->getDesc().byteSize / sizeof(ImDrawIdx));

		// copy and convert all vertices into a single contiguous buffer
		ImDrawVert *vtxDst = &m_VertexBufferData[0];
		ImDrawIdx * idxDst = &m_IndexBufferData[0];

		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList *cmdList = drawData->CmdLists[n];

			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}

		commandList->writeBuffer(m_VertexBuffer, &m_VertexBufferData[0], m_VertexBuffer->getDesc().byteSize);
		commandList->writeBuffer(m_IndexBuffer, &m_IndexBufferData[0], m_IndexBuffer->getDesc().byteSize);

		return true;
	}

	bool ImGuiRenderer::render(ImGuiViewport *     viewport, const nvrhi::GraphicsPipelineHandle &pipeline, const nvrhi::FramebufferHandle &framebuffer,
							   const vk::Semaphore waitSemaphore)
	{
		nvrhi::IDevice *device = Application::getGraphicsDevice();

		ImDrawData *drawData = viewport->DrawData;

		m_RenderCommandBuffer->_beginRecording();
		nvrhi::CommandListHandle commandList = m_RenderCommandBuffer->getActiveCommandBuffer();

		nvrhi::utils::ClearColorAttachment(m_RenderCommandBuffer->getActiveCommandBuffer(), framebuffer, 0, nvrhi::Color(1, 0, 1, 1));

		if (!updateGeometry(drawData))
		{
			m_RenderCommandBuffer->endRecording();
			return false;
		}

		// handle DPI scaling
		drawData->ScaleClipRects(drawData->FramebufferScale);

		struct PushConstants
		{
			tsm::vec2f Scale;
			tsm::vec2f Translate;
		} pushConstants;

		pushConstants.Scale.x     = 2.0f / drawData->DisplaySize.x;
		pushConstants.Scale.y     = 2.0f / drawData->DisplaySize.y;
		pushConstants.Translate.x = -1.0f - drawData->DisplayPos.x * pushConstants.Scale.x;
		pushConstants.Translate.y = -1.0f - drawData->DisplayPos.y * pushConstants.Scale.y;

		float fbWidth  = drawData->DisplaySize.x * drawData->FramebufferScale.x;
		float fbHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;

		// set up graphics state
		nvrhi::GraphicsState drawState;

		drawState.framebuffer = framebuffer;
		TST_ASSERT(drawState.framebuffer);

		drawState.pipeline = pipeline;

		drawState.viewport.viewports.push_back(nvrhi::Viewport(fbWidth, fbHeight));
		drawState.viewport.scissorRects.resize(1); // updated below

		nvrhi::VertexBufferBinding vbufBinding;
		vbufBinding.buffer = m_VertexBuffer;
		vbufBinding.slot   = 0;
		vbufBinding.offset = 0;
		drawState.vertexBuffers.push_back(vbufBinding);

		drawState.indexBuffer.buffer = m_IndexBuffer;
		drawState.indexBuffer.format = (sizeof(ImDrawIdx) == 2 ? nvrhi::Format::R16_UINT : nvrhi::Format::R32_UINT);
		drawState.indexBuffer.offset = 0;

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clip_off   = drawData->DisplayPos;       // (0,0) unless using multi-viewports
		ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// render command lists
		int vtxOffset = 0;
		int idxOffset = 0;
		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList *cmdList = drawData->CmdLists[n];
			for (int i = 0; i < cmdList->CmdBuffer.Size; i++)
			{
				const ImDrawCmd *pCmd = &cmdList->CmdBuffer[i];

				if (pCmd->UserCallback)
				{
					pCmd->UserCallback(cmdList, pCmd);
				}
				else
				{
					drawState.bindings = {getBindingSet(reinterpret_cast<nvrhi::ITexture *>(pCmd->TextureId))};
					TST_ASSERT(drawState.bindings[0]);

					// Project scissor/clipping rectangles into framebuffer space
					ImVec2 clipMin((pCmd->ClipRect.x - clip_off.x) * clip_scale.x, (pCmd->ClipRect.y - clip_off.y) * clip_scale.y);
					ImVec2 clipMax((pCmd->ClipRect.z - clip_off.x) * clip_scale.x, (pCmd->ClipRect.w - clip_off.y) * clip_scale.y);

					// Clamp to viewport as vkCmdSetScissor() won't accept values that are off bounds
					if (clipMin.x < 0.0f)
						clipMin.x = 0.0f;
					if (clipMin.y < 0.0f)
						clipMin.y = 0.0f;
					if (clipMax.x > fbWidth)
						clipMax.x = (float) fbWidth;
					if (clipMax.y > fbHeight)
						clipMax.y = (float) fbHeight;
					if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
						continue;

					drawState.viewport.scissorRects[0] = nvrhi::Rect(clipMin.x, clipMax.x, clipMin.y, clipMax.y);

					nvrhi::DrawArguments drawArguments;
					drawArguments.vertexCount         = pCmd->ElemCount;
					drawArguments.startIndexLocation  = pCmd->IdxOffset + idxOffset;
					drawArguments.startVertexLocation = pCmd->VtxOffset + vtxOffset;

					m_RenderCommandBuffer->getActiveCommandBuffer()->setGraphicsState(drawState);
					commandList->setPushConstants(&pushConstants, sizeof(PushConstants));
					commandList->drawIndexed(drawArguments);
				}
			}

			vtxOffset += cmdList->VtxBuffer.Size;
			idxOffset += cmdList->IdxBuffer.Size;
		}

		m_RenderCommandBuffer->_endRecording();

		// Wait for image to be available before rendering to it
		if (waitSemaphore)
			m_RenderCommandBuffer->_wait(waitSemaphore);

		m_RenderCommandBuffer->_submit();

		return true;
	}

	bool ImGuiRenderer::renderToSwapchain(ImGuiViewport *viewport, VulkanSwapChain *swapchain)
	{
		return render(viewport, getOrCreatePipeline(swapchain), swapchain->getCurrentFramebuffer(), swapchain->getAcquiredImageSemaphore());
	}

	void ImGuiRenderer::backbufferResizing()
	{
		m_PipelineCache.clear();
	}
}
