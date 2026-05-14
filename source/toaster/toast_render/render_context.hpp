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
#include "toast_gpu/vk/vk_mesh.hpp"

namespace toaster::gpu
{
	class VKInstance;
	class VKPhysicalDevice;
	class VKLogicalDevice;
}

namespace toaster::render
{
	class Globals;

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

		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createObjectRef(TArgs &&... p_args) const -> RefPtr<TObj>
		{
			return m_logicalDevice->alloc<TObj>(std::forward<TArgs>(p_args)...);
		}

		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createObject(TArgs &&... p_args) const -> TObj
		{
			return TObj{m_logicalDevice, std::forward<TArgs>(p_args)...};
		}

		template<typename TUBOStruct>
		[[nodiscard]] auto createUniformBuffer() const -> RefPtr<gpu::VKUniformBuffer>
		{
			constexpr uint64 ubo_size{sizeof(TUBOStruct)};
			return m_logicalDevice->alloc<gpu::VKUniformBuffer>(ubo_size);
		}

		template<typename TUBOStruct>
		[[nodiscard]] auto createUniformBuffers(uint32 p_count) const -> RefPtr<gpu::VKUniformBufferPFF>
		{
			constexpr uint64 ubo_size{sizeof(TUBOStruct)};
			return m_logicalDevice->alloc<gpu::VKUniformBufferPFF>(ubo_size, p_count);
		}

		[[nodiscard]] auto createEnvironmentMap(const io::filesystem::Path &p_path) const -> RefPtr<gpu::VKTexture3D>;

		#pragma region render logic
		auto beginRendering(gpu::VKCommandBuffer &           p_command_buffer, const gpu::RenderingInfo &p_rendering_info, uint32 p_frame_index,
							const RefPtr<gpu::VKRenderPass> &p_render_pass) const -> void;
		auto endRendering(gpu::VKCommandBuffer &p_command_buffer, const gpu::RenderingInfo &p_rendering_info) const -> void;

		auto beginCompute(gpu::VKCommandBuffer &p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKComputePass> &p_compute_pass) const -> void;
		auto dispatchCompute(gpu::VKCommandBuffer &         p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKComputePass> &p_compute_pass,
							 const RefPtr<gpu::VKMaterial> &p_material, uint32       p_work_group_x, uint32 p_work_group_y, uint32 p_work_group_z) const -> void;

		auto renderGeometry(gpu::VKCommandBuffer &             p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
							const RefPtr<gpu::VKVertexBuffer> &p_vertex_buffer, const RefPtr<gpu::VKIndexBuffer> &p_index_buffer, uint32 p_index_count,
							const RefPtr<gpu::VKMaterial> &    p_material, const glm::mat4 &p_transform) const -> void;

		auto renderFullscreenQuad(gpu::VKCommandBuffer &         p_command_buffer, uint32 p_frame_index, const RefPtr<gpu::VKPipeline> &p_pipeline,
								  const RefPtr<gpu::VKMaterial> &p_material) const -> void;

		auto renderMesh(gpu::VKCommandBuffer &         p_command_buffer, uint32     p_frame_index, const RefPtr<gpu::VKMesh> &p_mesh, uint32 p_submesh_index,
						const RefPtr<gpu::VKPipeline> &p_pipeline, const glm::mat4 &p_transform) const -> void;

		auto renderMesh(gpu::VKCommandBuffer &         p_command_buffer, uint32     p_frame_index, const RefPtr<gpu::VKMesh> &  p_mesh, uint32 p_submesh_index,
						const RefPtr<gpu::VKPipeline> &p_pipeline, const glm::mat4 &p_transform, const RefPtr<gpu::VKMaterial> &p_override_material) const -> void;
		#pragma endregion

	private:
		RenderContextSpecInfo m_specInfo{};

		gpu::VKInstance *      m_backendInstance{nullptr};
		gpu::VKPhysicalDevice *m_physicalDevice{nullptr};
		gpu::VKLogicalDevice * m_logicalDevice{nullptr};

		Globals *m_globals{nullptr};
	};
}
