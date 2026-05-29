#pragma once

#include "toast_render.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_render_attachment.hpp"
#include "toast_gpu/vk/vk_render_pass.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

namespace toaster::render
{
	class Globals;
	class Material;
	class MeshData;

	struct TST_RENDER_API RenderContextSpecInfo
	{
		std::unordered_set<String> instanceExtensions; // Get with Window::getRequiredInstanceExtensions()

		io::filesystem::Path binaryDir;
		bool                 printDebugInfo{true};
		bool                 createGlobals{true}; // You will have to compile the shaders for this to work
	};

	class TST_RENDER_API RenderContext
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

		[[nodiscard]] auto getCurrentFrameIndex() const -> uint32;
		auto               setCurrentFrameIndex(uint32 p_index) -> void;
		auto               performGarbageCollection() const -> void;

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto create(TArgs &&... p_args) -> RefPtr<TObj>
		{
			return make_reference<TObj>(this, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the logical device into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPU(TArgs &&... p_args) const -> RefPtr<TObj>
		{
			return make_reference<TObj>(m_logicalDevice, std::forward<TArgs>(p_args)...);
		}

		template<typename TVertex>
		[[nodiscard]] auto createVertexBuffer(const std::vector<TVertex> &p_vertices) const -> gpu::VertexBufferHandle
		{
			const uint64 vbo_size{sizeof(TVertex) * p_vertices.size()};
			return createGPU<gpu::VKVertexBuffer>(static_cast<const void *>(p_vertices.data()), vbo_size);
		}

		template<typename TIndex>
		[[nodiscard]] auto createIndexBuffer(const std::vector<TIndex> &p_indices) const -> gpu::IndexBufferHandle
		{
			const uint64 ibo_size{sizeof(TIndex) * p_indices.size()};
			return createGPU<gpu::VKIndexBuffer>(static_cast<const void *>(p_indices.data()), ibo_size);
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

		[[nodiscard]] auto createAttachmentImage(uint32     p_width, uint32 p_height, vk::ImageAspectFlags p_image_aspect_flags,
												 vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageHandle;
		[[nodiscard]] auto createMultisampleAttachmentImage(uint32     p_width, uint32 p_height, vk::ImageAspectFlags p_image_aspect_flags,
															vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageHandle;
		[[nodiscard]] auto createAttachmentTexture(uint32     p_width, uint32 p_height, vk::ImageAspectFlags p_image_aspect_flags,
												   vk::Format p_format = vk::Format::eUndefined) const -> gpu::Texture2DHandle;

		[[nodiscard]] auto createEnvironmentMap(const io::filesystem::Path &p_path) const -> gpu::Texture3DHandle;
		[[nodiscard]] auto createEnvironmentMap(const gpu::TextureSpecInfo &p_spec_info, const Buffer &p_data) const -> gpu::Texture3DHandle;
		[[nodiscard]] auto createDiffuseIrradianceMap(const gpu::Texture3DHandle &p_environment_map) const -> gpu::Texture3DHandle;

		#pragma region render logic
		auto beginRendering(gpu::VKCommandBuffer *p_command_buffer, const gpu::RenderingInfo &p_rendering_info, gpu::VKRenderPass *p_render_pass,
							uint32                p_frame_index = UINT32_MAX) const -> void;
		auto endRendering(gpu::VKCommandBuffer *p_command_buffer, const gpu::RenderingInfo &p_rendering_info) const -> void;

		auto beginCompute(gpu::VKCommandBuffer *p_command_buffer, gpu::VKComputePass *p_compute_pass, uint32 p_frame_index = UINT32_MAX) const -> void;
		auto dispatchCompute(gpu::VKCommandBuffer *p_command_buffer, const gpu::VKComputePass *p_compute_pass, Material *p_material, uint32 p_work_group_x,
							 uint32                p_work_group_y, uint32                      p_work_group_z, uint32    p_frame_index = UINT32_MAX) const -> void;

		auto renderGeometry(gpu::VKCommandBuffer *p_command_buffer, gpu::VKPipeline *p_pipeline, gpu::VKVertexBuffer *p_vertex_buffer, gpu::VKIndexBuffer *p_index_buffer,
							uint32                p_index_count, Material *p_material, const tsm::float4x4 &p_transform, uint32 p_frame_index = UINT32_MAX) const -> void;

		auto renderFullscreenQuad(gpu::VKCommandBuffer *p_command_buffer, gpu::VKPipeline *p_pipeline, Material *p_material,
								  uint32                p_frame_index = UINT32_MAX) const -> void;

		auto renderMesh(gpu::VKCommandBuffer *p_command_buffer, const MeshData *p_mesh, uint32 p_submesh_index, gpu::VKPipeline *p_pipeline, const tsm::float4x4 &p_transform,
						uint32                p_frame_index = UINT32_MAX) const -> void;

		auto renderMesh(gpu::VKCommandBuffer *p_command_buffer, const MeshData *p_mesh, uint32 p_submesh_index, gpu::VKPipeline *p_pipeline, const tsm::float4x4 &p_transform,
						Material *            p_override_material, uint32       p_frame_index = UINT32_MAX) const -> void;
		#pragma endregion

	private:
		RenderContextSpecInfo m_specInfo{};

		OwningPtr<gpu::VKInstance>       m_backendInstance{nullptr};
		OwningPtr<gpu::VKPhysicalDevice> m_physicalDevice{nullptr};
		OwningPtr<gpu::VKLogicalDevice>  m_logicalDevice{nullptr};

		OwningPtr<Globals> m_globals{nullptr};
	};
}
