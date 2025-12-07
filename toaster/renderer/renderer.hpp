#pragma once

#include <unordered_set>

#include "renderer_config_info.hpp"
#include "render_command_queue.hpp"
#include "core/core.hpp"
#include "gpu/shader.hpp"
#include "gpu/pipeline.hpp"
#include "gpu/compute_pipeline.hpp"
#include "gpu/compute_pass.hpp"
#include "gpu/material.hpp"
#include "gpu/vertex_buffer.hpp"
#include "gpu/index_buffer.hpp"
#include "gpu/render_pass.hpp"
#include "core/ref_ptr.hpp"
#include "platform/render_thread.hpp"

namespace tst
{
	// Static renderer interface
	class Renderer
	{
	public:
		template<typename FuncT>
		static void submit(FuncT &&func)
		{
			auto renderCmd = [](void *ptr)
			{
				auto pFunc = static_cast<FuncT *>(ptr);
				(*pFunc)();

				pFunc->~FuncT();
			};
			auto storageBuffer = _getRenderCommandQueue().alloc(renderCmd, sizeof(func));
			new(storageBuffer) FuncT(std::forward<FuncT>(func));
		}

		static void waitAndRender(RenderThread *renderThread);
		static void swapQueues();

		static void   renderThreadFunc(RenderThread *renderThread);
		static uint32 getRenderQueueIndex();
		static uint32 getRenderQueueSubmissionIndex();

		static void beginFrame();
		static void endFrame();

		static uint32_t getCurrentFrameIndex();  // From the application
		static uint32_t _getCurrentFrameIndex(); // From the window's swapchain

		// Initialization
		static EError init();
		static void   terminate();

		static void                      setConfigInfo(const RendererConfigInfo &configInfo);
		static const RendererConfigInfo &getConfigInfo();

		static void beginRenderPass(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<RenderPass> renderPass, bool explicitClear = false);
		static void endRenderPass(RefPtr<CommandBuffer> renderCommandBuffer);

		static void beginComputePass(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<ComputePass> computePass);
		static void endComputePass(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<ComputePass> computePass);
		static void dispatchCompute(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<ComputePass> computePass, RefPtr<Material> material, const tsm::vec3ui &workGroups,
									Buffer                constants = Buffer());

		static void renderGeometry(RefPtr<CommandBuffer> renderCommandBuffer, RefPtr<Pipeline> pipeline, RefPtr<Material> material, RefPtr<VertexBuffer> vertexBuffer,
								   RefPtr<IndexBuffer>   indexBuffer, const tsm::mat4x4f &     transform, uint32_t        indexCount = 0);

		static const std::unordered_map<std::string, std::string> &getGlobalShaderMacros();
		static void                                                acknowledgeParsedGlobalMacros(const std::unordered_set<std::string> &macros, RefPtr<Shader> shader);
		static void                                                setMacroInShader(RefPtr<Shader> shader, const std::string &name, const std::string &value = "");
		static void                                                setGlobalMacroInShaders(const std::string &name, const std::string &value = "");

		static void registerShaderDependency(RefPtr<Shader> shader, RefPtr<ComputePipeline> computePipeline);
		static void registerShaderDependency(RefPtr<Shader> shader, RefPtr<Pipeline> pipeline);
		static void registerShaderDependency(RefPtr<Shader> shader, RefPtr<Material> material);

		static RefPtr<Texture2D> getWhiteTexture();
		static RefPtr<Texture2D> getBlackTexture();

		static RefPtr<ShaderLibrary> getShaderLibrary();

		static bool updateDirtyShaders();

		static RefPtr<Sampler> getClampSampler();
		static RefPtr<Sampler> getPointSampler();
		static RefPtr<Sampler> getDefaultSampler() { return getClampSampler(); }

	private:
		static RenderCommandQueue &_getRenderCommandQueue();
	};
}
