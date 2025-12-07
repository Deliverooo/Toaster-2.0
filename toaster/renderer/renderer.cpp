#include "renderer.hpp"

#include <shared_mutex>

#include "gpu/vulkan/vulkan_swapchain.hpp"
#include "nvrhi/utils.h"
#include "platform/application.hpp"

namespace std
{
	template<>
	struct hash<tst::CachedPtr<tst::Shader> >
	{
		size_t operator()(const tst::CachedPtr<tst::Shader> &shader) const noexcept
		{
			return shader->getHash();
		}
	};
}

namespace tst
{
	struct RendererData
	{
		std::unordered_map<std::string, std::string> globalShaderMacros;

		RefPtr<ShaderLibrary> shaderLibrary;

		RefPtr<Texture2D> whiteTexture;
		RefPtr<Texture2D> blackTexture;

		// Default samplers
		RefPtr<Sampler> samplerClamp = nullptr;
		RefPtr<Sampler> samplerPoint = nullptr;
	};

	struct ShaderDependencies
	{
		std::vector<RefPtr<ComputePipeline> > ComputePipelines;
		std::vector<RefPtr<Pipeline> >        Pipelines;
		std::vector<RefPtr<Material> >        Materials;
	};

	static std::unordered_map<size_t, ShaderDependencies> s_ShaderDependencies;
	static std::shared_mutex s_ShaderDependenciesMutex; // ShaderDependencies can be accessed (and modified) from multiple threads, hence require synchronization

	struct GlobalShaderInfo
	{
		std::unordered_map<std::string, std::unordered_map<size_t, CachedPtr<Shader> > > shaderGlobalMacrosMap;
		std::unordered_set<CachedPtr<Shader> >                                           dirtyShaders;
	};

	static GlobalShaderInfo s_globalShaderInfo;

	static RendererData *s_data = nullptr;

	static RendererConfigInfo s_rendererConfigInfo;

	constexpr uint32           s_renderCommandQueueCount = 2;
	RenderCommandQueue *       s_renderCommandQueue[s_renderCommandQueueCount];
	static std::atomic<uint32> s_renderCommandQueueSubmissionIndex = 0;

	EError Renderer::init()
	{
		s_data = tnew RendererData();

		s_renderCommandQueue[0] = tnew RenderCommandQueue();
		s_renderCommandQueue[1] = tnew RenderCommandQueue();

		s_rendererConfigInfo.maxFramesInFlight = tsm::min<uint32>(s_rendererConfigInfo.maxFramesInFlight,
																  Application::getInstance().getMainWindow().getSwapchain()->getBackBufferCount());

		s_data->shaderLibrary = make_reference<ShaderLibrary>();

		setGlobalMacroInShaders("__TST_TEST_MACRO", "0");

		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/ImGui.glsl");
		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/shader.glsl");
		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/Renderer2D.glsl");
		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/Renderer2D_Circle.glsl");
		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/Renderer2D_Line.glsl");
		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/LinearSample.glsl");
		s_data->shaderLibrary->load("C:/dev/Toaster/tools/sandbox/res/shaders/LinearSampleUInt.glsl");

		Application::getInstance().getRenderThread().pump();

		uint32_t        whiteTextureData = 0xffffffff;
		TextureSpecInfo spec;
		spec.DebugName       = "Renderer-WhiteTexture";
		spec.Format          = ImageFormat::RGBA;
		spec.Width           = 1;
		spec.Height          = 1;
		s_data->whiteTexture = make_reference<Texture2D>(spec, Buffer(&whiteTextureData, sizeof(uint32_t)));

		constexpr uint32_t blackTextureData = 0xff000000;
		spec.DebugName                      = "Renderer-BlackTexture";
		s_data->blackTexture                = make_reference<Texture2D>(spec, Buffer(&blackTextureData, sizeof(uint32_t)));

		return EError::eOk;
	}

	void Renderer::terminate()
	{
		Application::getDeviceManager()->GetDevice()->waitForIdle();

		delete s_data;

		delete s_renderCommandQueue[0];
		delete s_renderCommandQueue[1];
	}

	void Renderer::waitAndRender(RenderThread *renderThread)
	{
		renderThread->waitAndSet(RenderThread::EState::eKick, RenderThread::EState::eBusy);

		s_renderCommandQueue[getRenderQueueIndex()]->executeCommands();

		renderThread->set(RenderThread::EState::eIdle);
	}

	void Renderer::swapQueues()
	{
		s_renderCommandQueueSubmissionIndex = (s_renderCommandQueueSubmissionIndex + 1) % s_renderCommandQueueCount;
	}

	void Renderer::renderThreadFunc(RenderThread *renderThread)
	{
		while (renderThread->isRunning())
		{
			waitAndRender(renderThread);
		}
	}

	uint32 Renderer::getRenderQueueIndex()
	{
		return (s_renderCommandQueueSubmissionIndex + 1) % s_renderCommandQueueCount;
	}

	uint32 Renderer::getRenderQueueSubmissionIndex()
	{
		return s_renderCommandQueueSubmissionIndex;
	}

	void Renderer::registerShaderDependency(RefPtr<Shader> shader, RefPtr<ComputePipeline> computePipeline)
	{
		std::scoped_lock lock(s_ShaderDependenciesMutex);
		s_ShaderDependencies[shader->getHash()].ComputePipelines.push_back(computePipeline);
	}

	void Renderer::registerShaderDependency(RefPtr<Shader> shader, RefPtr<Pipeline> pipeline)
	{
		std::scoped_lock lock(s_ShaderDependenciesMutex);
		s_ShaderDependencies[shader->getHash()].Pipelines.push_back(pipeline);
	}

	void Renderer::registerShaderDependency(RefPtr<Shader> shader, RefPtr<Material> material)
	{
		std::scoped_lock lock(s_ShaderDependenciesMutex);
		s_ShaderDependencies[shader->getHash()].Materials.push_back(material);
	}

	void Renderer::beginFrame()
	{
	}

	void Renderer::endFrame()
	{
	}

	RefPtr<Texture2D> Renderer::getWhiteTexture()
	{
		return s_data->whiteTexture;
	}

	RefPtr<Texture2D> Renderer::getBlackTexture()
	{
		return s_data->blackTexture;
	}

	uint32_t Renderer::getCurrentFrameIndex()
	{
		return Application::getInstance().getCurrentFrameIndex();
	}

	uint32_t Renderer::_getCurrentFrameIndex()
	{
		return Application::getInstance().getMainWindow().getSwapchain()->getCurrentBackBufferIndex();
	}

	RenderCommandQueue &Renderer::_getRenderCommandQueue()
	{
		return *s_renderCommandQueue[s_renderCommandQueueSubmissionIndex];
	}

	const RendererConfigInfo &Renderer::getConfigInfo()
	{
		return s_rendererConfigInfo;
	}

	void Renderer::setConfigInfo(const RendererConfigInfo &configInfo)
	{
		s_rendererConfigInfo = configInfo;
	}

	const std::unordered_map<std::string, std::string> &Renderer::getGlobalShaderMacros()
	{
		return s_data->globalShaderMacros;
	}

	void Renderer::acknowledgeParsedGlobalMacros(const std::unordered_set<std::string> &macros, RefPtr<Shader> shader)
	{
		for (const std::string &macro: macros)
		{
			s_globalShaderInfo.shaderGlobalMacrosMap[macro][shader->getHash()] = shader;
		}
	}

	void Renderer::setMacroInShader(RefPtr<Shader> shader, const std::string &name, const std::string &value)
	{
		shader->setShaderMacro(name, value);
		s_globalShaderInfo.dirtyShaders.emplace(shader.get());
	}

	void Renderer::setGlobalMacroInShaders(const std::string &name, const std::string &value)
	{
		if (s_data->globalShaderMacros.find(name) != s_data->globalShaderMacros.end())
		{
			if (s_data->globalShaderMacros.at(name) == value)
				return;
		}

		s_data->globalShaderMacros[name] = value;

		if (s_globalShaderInfo.shaderGlobalMacrosMap.find(name) == s_globalShaderInfo.shaderGlobalMacrosMap.end())
		{
			TST_WARN_TAG("Renderer", "No shaders with {} macro found", name);
			return;
		}

		TST_ASSERT(s_globalShaderInfo.shaderGlobalMacrosMap.find(name) != s_globalShaderInfo.shaderGlobalMacrosMap.end());
		for (auto &[hash, shader]: s_globalShaderInfo.shaderGlobalMacrosMap.at(name))
		{
			TST_ASSERT(shader.isValid());
			s_globalShaderInfo.dirtyShaders.emplace(shader);
		}
	}

	RefPtr<ShaderLibrary> Renderer::getShaderLibrary()
	{
		return s_data->shaderLibrary;
	}

	void Renderer::beginRenderPass(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<RenderPass> renderPass, bool explicitClear)
	{
		Renderer::submit([renderCommandBuffer, renderPass, explicitClear]() mutable
		{
			RefPtr<Pipeline>    pipeline    = renderPass->getSpecInfo().pipeline;
			RefPtr<Framebuffer> framebuffer = pipeline->getSpecInfo().targetFramebuffer;

			if (explicitClear || framebuffer->getSpecification().ClearColorOnLoad || framebuffer->getSpecification().ClearDepthOnLoad)
			{
				const auto &clearValues = framebuffer->getClearValues();

				if (explicitClear || framebuffer->getSpecification().ClearColorOnLoad)
				{
					for (size_t i = 0; i < framebuffer->getColorAttachmentCount(); i++)
					{
						nvrhi::Color color = nvrhi::Color(clearValues[i].Color.float32[0], clearValues[i].Color.float32[1], clearValues[i].Color.float32[2],
														  clearValues[i].Color.float32[3]);

						nvrhi::utils::ClearColorAttachment(renderCommandBuffer->getActiveCommandBuffer(), framebuffer->getHandle(), i, color);
					}
				}

				if (explicitClear || framebuffer->getSpecification().ClearDepthOnLoad)
				{
					if (framebuffer->hasDepthAttachment())
					{
						const auto &depthStencil = clearValues[clearValues.size() - 1].DepthStencil;
						nvrhi::utils::ClearDepthStencilAttachment(renderCommandBuffer->getActiveCommandBuffer(), framebuffer->getHandle(), depthStencil.Depth,
																  depthStencil.Stencil);
					}
				}
			}

			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();

			nvrhi::GraphicsState &graphicsState = renderCommandBuffer->getGraphicsState();
			graphicsState.pipeline              = pipeline->getHandle();
			TST_ASSERT(graphicsState.pipeline);
			graphicsState.framebuffer = framebuffer->getHandle();
			TST_ASSERT(graphicsState.framebuffer);

			// Viewport and scissor
			float fbWidth                       = (float) framebuffer->getWidth();
			float fbHeight                      = (float) framebuffer->getHeight();
			graphicsState.viewport.viewports    = {nvrhi::Viewport(fbWidth, fbHeight)};
			graphicsState.viewport.scissorRects = {nvrhi::Rect(fbWidth, fbHeight)};

			// graphicsState.lineWidth = 0.0f;
			// if (renderPass->getPipeline()->isDynamicLineWidth())
			// graphicsState.lineWidth = renderPass->GetPipeline()->GetSpecification().LineWidth;

			renderPass->prepare();
			auto bindingSets       = renderPass->getBindingSets(Renderer::_getCurrentFrameIndex());
			graphicsState.bindings = bindingSets;

			renderCommandBuffer->_commitGraphicsState();
		});
	}

	void Renderer::endRenderPass(RefPtr<CommandBuffer> renderCommandBuffer)
	{
	}

	void Renderer::beginComputePass(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<ComputePass> computePass)
	{
		TST_ASSERT(computePass);

		Renderer::submit([renderCommandBuffer, computePass]() mutable
		{
			RefPtr<ComputePipeline> pipeline = computePass->GetPipeline();

			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();

			nvrhi::ComputeState &computeState = renderCommandBuffer->getComputeState();
			computeState.pipeline             = pipeline->getHandle();
			TST_ASSERT(computeState.pipeline);

			computePass->Prepare();

			auto bindingSets      = computePass->GetBindingSets(Renderer::_getCurrentFrameIndex());
			computeState.bindings = bindingSets;

			renderCommandBuffer->_commitComputeState();
		});
	}

	void Renderer::endComputePass(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<ComputePass> computePass)
	{
	}

	void Renderer::dispatchCompute(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<ComputePass> computePass, RefPtr<Material> material, const tsm::vec3ui &workGroups,
								   Buffer                constants)
	{
		Buffer pushConstantBuffer;
		if (constants)
			pushConstantBuffer = Buffer::copy(constants);

		Renderer::submit([renderCommandBuffer, computePass, material, workGroups, pushConstantBuffer]() mutable
		{
			const uint32_t           frameIndex  = Renderer::_getCurrentFrameIndex();
			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();

			nvrhi::ComputeState &computeState = renderCommandBuffer->getComputeState();

			// Bind material descriptor set if exists
			if (material)
			{
				material->prepare();
				auto bindingSet = material->getBindingSet(frameIndex);
				if (bindingSet)
				{
					if (computeState.bindings.empty())
						computeState.bindings.resize(1);

					computeState.bindings[0] = bindingSet;
				}
			}

			renderCommandBuffer->_commitComputeState();

			if (pushConstantBuffer)
			{
				commandList->setPushConstants(pushConstantBuffer.data, pushConstantBuffer.size);
				pushConstantBuffer.release();
			}

			commandList->dispatch(workGroups.x, workGroups.y, workGroups.z);
		});
	}

	void Renderer::renderGeometry(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<Pipeline> pipeline, RefPtr<Material> material, RefPtr<VertexBuffer> vertexBuffer,
								  RefPtr<IndexBuffer>   indexBuffer, const tsm::mat4x4f &     transform, uint32_t        indexCount /*= 0*/)
	{
		Renderer::submit([renderCommandBuffer, pipeline, material, vertexBuffer, indexBuffer, transform, indexCount]() mutable
		{
			nvrhi::CommandListHandle commandList = renderCommandBuffer->getActiveCommandBuffer();

			nvrhi::GraphicsState &graphicsState = renderCommandBuffer->getGraphicsState();

			nvrhi::VertexBufferBinding vertexBufferBinding;
			vertexBufferBinding.buffer  = vertexBuffer->getBufferHandle();
			vertexBufferBinding.slot    = 0;
			vertexBufferBinding.offset  = 0;
			graphicsState.vertexBuffers = {vertexBufferBinding};

			nvrhi::IndexBufferBinding indexBufferBinding;
			indexBufferBinding.buffer = indexBuffer->getBufferHandle();
			indexBufferBinding.format = nvrhi::Format::R32_UINT;
			indexBufferBinding.offset = 0;
			graphicsState.indexBuffer = indexBufferBinding;

			material->prepare();
			auto bindingSet           = material->getBindingSet(Renderer::_getCurrentFrameIndex());
			graphicsState.bindings[0] = bindingSet;

			renderCommandBuffer->_commitGraphicsState();

			commandList->setPushConstants(&transform, sizeof(tsm::mat4x4f));

			nvrhi::DrawArguments drawArgs;
			drawArgs.vertexCount         = indexCount;
			drawArgs.startIndexLocation  = 0;
			drawArgs.startVertexLocation = 0;
			commandList->drawIndexed(drawArgs);
		});
	}

	bool Renderer::updateDirtyShaders()
	{
		const bool updatedAnyShaders = s_globalShaderInfo.dirtyShaders.size();
		for (CachedPtr<Shader> shader: s_globalShaderInfo.dirtyShaders)
		{
			TST_ASSERT(shader.isValid());
			shader->_reload(true);
		}
		s_globalShaderInfo.dirtyShaders.clear();

		return updatedAnyShaders;
	}

	RefPtr<Sampler> Renderer::getClampSampler()
	{
		if (!s_data->samplerClamp)
			s_data->samplerClamp = make_reference<Sampler>();

		return s_data->samplerClamp;
	}

	RefPtr<Sampler> Renderer::getPointSampler()
	{
		if (!s_data->samplerPoint)
		{
			SamplerSpecification spec;
			spec.MinFilter       = spec.MagFilter = spec.MipFilter = false;
			s_data->samplerPoint = make_reference<Sampler>(spec);
		}

		return s_data->samplerPoint;
	}
}
