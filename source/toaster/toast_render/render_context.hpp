#pragma once

#include "toaster_macros.hpp"
#include "toast_lib/ptr.hpp"

#include <unordered_set>

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

namespace toaster::gpu
{
	class VKInstance;
	class VKPhysicalDevice;
	class VKLogicalDevice;
}

namespace toaster::render
{
	class Globals;
	class Material;
	class Mesh;

	struct TST_API RenderContextSpecInfo
	{
		std::unordered_set<String> instanceExtensions; // Get with Window::getRequiredInstanceExtensions()

		io::filesystem::Path binaryDir;
	};

	class TST_API RenderContext
	{
	public:
		static constexpr uint32 maxFramesInFlight{3u};

		RenderContext(const RenderContextSpecInfo &p_spec_info);
		~RenderContext();

		[[nodiscard]] auto getBackendInstance() const -> gpu::VKInstance *;
		[[nodiscard]] auto getPhysicalDevice() const -> gpu::VKPhysicalDevice *;
		[[nodiscard]] auto getLogicalDevice() const -> gpu::VKLogicalDevice *;

		[[nodiscard]] auto getGlobals() const -> const Globals *;

		#pragma region gpu operations
		auto gpuWaitIdle() const -> void;
		#pragma endregion

		auto setCurrentFrameIndex(uint32 p_index) -> void;
		auto performGarbageCollection() -> void;

		// If you care about not getting crashes when allocating gpu objects mid-frame, you should use this instead of make_reference<Type>(ctx, ...)
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto create(TArgs &&... p_args)  -> RefPtr<TObj>
		{
			return allocate_reference<TObj>([this](TObj *p_ptr) -> void
			{
				LOG_ERROR("Deleting: {}", typeid(TObj).name());
				auto deleter{
					[p_ptr]() -> void
					{
						delete p_ptr;
					}
				};
				m_pendingDeletions[m_currentFrameIndex].emplace_back(std::move(deleter));
			}, this, std::forward<TArgs>(p_args)...);
		}

		// If you care about not getting crashes when allocating gpu objects mid-frame, you should use this instead of make_reference<Type>(ctx, ...)
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPU(TArgs &&... p_args) const -> RefPtr<TObj>
		{
			LOG_ERROR("Deleting: {}", typeid(TObj).name());

			return allocate_reference<TObj>([this](TObj *p_ptr) -> void
			{
				auto deleter{
					[p_ptr]() -> void
					{
						delete p_ptr;
					}
				};
				m_pendingDeletions[m_currentFrameIndex].emplace_back(std::move(deleter));
			}, m_logicalDevice, std::forward<TArgs>(p_args)...);
		}

		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPUObjectRef(TArgs &&... p_args) const -> RefPtr<TObj>
		{
			return create<TObj>(std::forward<TArgs>(p_args)...);
		}

		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPUObject(TArgs &&... p_args) const -> TObj
		{
			return TObj{m_logicalDevice, std::forward<TArgs>(p_args)...};
		}

		template<typename TUBOStruct>
		[[nodiscard]] auto createUniformBuffer() const -> gpu::UniformBufferHandle
		{
			constexpr uint64 ubo_size{sizeof(TUBOStruct)};
			return createGPU<gpu::VKUniformBuffer>(ubo_size);
		}

		template<typename TUBOStruct>
		[[nodiscard]] auto createUniformBuffers(uint32 p_count) const -> gpu::UniformBufferPFFHandle
		{
			constexpr uint64 ubo_size{sizeof(TUBOStruct)};
			return createGPU<gpu::VKUniformBufferPFF>(ubo_size, p_count);
		}

		[[nodiscard]] auto createEnvironmentMap(const io::filesystem::Path &p_path) const -> gpu::Texture3DHandle;

		#pragma region render logic
		auto beginRendering(gpu::VKCommandBuffer &p_command_buffer, const gpu::RenderingInfo &p_rendering_info, uint32 p_frame_index,
							gpu::VKRenderPass *   p_render_pass) const -> void;
		auto endRendering(gpu::VKCommandBuffer &p_command_buffer, const gpu::RenderingInfo &p_rendering_info) const -> void;

		auto beginCompute(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, gpu::VKComputePass *p_compute_pass) const -> void;
		auto dispatchCompute(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, const gpu::VKComputePass *p_compute_pass, Material *p_material,
							 uint32                p_work_group_x, uint32   p_work_group_y, uint32                   p_work_group_z) const -> void;

		auto renderGeometry(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, gpu::VKPipeline *p_pipeline, gpu::VKVertexBuffer *p_vertex_buffer,
							gpu::VKIndexBuffer *  p_index_buffer, uint32   p_index_count, Material *       p_material, const glm::mat4 &    p_transform) const -> void;

		auto renderFullscreenQuad(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, gpu::VKPipeline *p_pipeline, Material *p_material) const -> void;

		auto renderMesh(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, const Mesh *p_mesh, uint32 p_submesh_index, gpu::VKPipeline *p_pipeline,
						const glm::mat4 &     p_transform) const -> void;

		auto renderMesh(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, const Mesh *p_mesh, uint32 p_submesh_index, gpu::VKPipeline *p_pipeline,
						const glm::mat4 &     p_transform, Material *  p_override_material) const -> void;
		#pragma endregion

	private:
		RenderContextSpecInfo m_specInfo{};

		OwningPtr<gpu::VKInstance>       m_backendInstance{nullptr};
		OwningPtr<gpu::VKPhysicalDevice> m_physicalDevice{nullptr};
		OwningPtr<gpu::VKLogicalDevice>  m_logicalDevice{nullptr};

		OwningPtr<Globals> m_globals{nullptr};

		mutable std::vector<std::deque<std::function<void()> > > m_pendingDeletions;
		mutable std::vector<std::deque<std::function<void()> > > m_pendingResourceUpdates;
		mutable uint32                                           m_currentFrameIndex{0};
	};
}
