#pragma once

#include "render_attachment.hpp"
#include "shader_compiler.hpp"
#include "toast_render.hpp"
#include "toast_gpu/buffer_layout.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_index_buffer.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_gpu/vk/vk_texture.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_gpu/vk/vk_vertex_buffer.hpp"

namespace toaster::render
{
	class Globals;
	class Material;
	class MeshData;
	class Renderer2D;
	class RenderPass;
	class ComputePass;
	class Image;
	class ShaderCompiler;

	enum class ESamplerType
	{
		eDefault, eNearest
	};

	struct TST_RENDER_API RenderContextSpecInfo
	{
		std::unordered_set<String> instanceExtensions; // Get with Window::getRequiredInstanceExtensions()

		io::filesystem::Path sdkDir;
		bool                 printDebugInfo{true};
		bool                 createGlobals{true}; // You will have to compile the shaders for this to work
	};

	class TST_RENDER_API RenderContext
	{
	public:
		static constexpr uint32 maxFramesInFlight{3u};

		static inline const gpu::VertexBufferLayout fullscreenQuadVbl{{gpu::EBufferDataType::eFloat3, "a_Position"}, {gpu::EBufferDataType::eFloat2, "a_TexCoord"}};
		static inline const gpu::VertexBufferLayout meshVbl{
			{gpu::EBufferDataType::eFloat3, "a_Position"},
			{gpu::EBufferDataType::eFloat3, "a_Normal"},
			{gpu::EBufferDataType::eFloat3, "a_Tangent"},
			{gpu::EBufferDataType::eFloat3, "a_Bitangent"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};

		RenderContext(const RenderContextSpecInfo &p_spec_info);
		~RenderContext();

		[[nodiscard]] auto getBackendInstance() const -> gpu::VKInstance *;
		[[nodiscard]] auto getPhysicalDevice() const -> gpu::VKPhysicalDevice *;
		[[nodiscard]] auto getLogicalDevice() const -> gpu::VKLogicalDevice *;

		[[nodiscard]] auto getDescriptorHeap() const -> gpu::VKDescriptorHeap *;

		[[nodiscard]] auto getGlobals() const -> const Globals *;

		#pragma region gpu operations
		auto gpuWaitIdle() const -> void;
		#pragma endregion

		[[nodiscard]] auto getCurrentFrameIndex() const -> uint32;
		auto               setCurrentFrameIndex(uint32 p_index) -> void;
		auto               performGarbageCollection() const -> void;

		[[nodiscard]] auto getCurrentSwapchainCommandBuffer() const -> gpu::VKCommandBuffer *;
		auto               setCurrentSwapchainCommandBuffer(gpu::VKCommandBuffer *p_cmd) -> void; // ONLY THE APPLICATION SHOULD USE TS...

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createRef(TArgs &&... p_args) -> RefPtr<TObj>
		{
			return make_reference<TObj>(*this, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createUnique(TArgs &&... p_args) -> UniquePtr<TObj>
		{
			return toaster::make_unique<TObj>(*this, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the logical device into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPURef(TArgs &&... p_args) const -> RefPtr<TObj>
		{
			return make_reference<TObj>(m_logicalDevice, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPUUnique(TArgs &&... p_args) -> UniquePtr<TObj>
		{
			return toaster::make_unique<TObj>(m_logicalDevice, std::forward<TArgs>(p_args)...);
		}

		template<typename TVertex>
		[[nodiscard]] auto createVertexBuffer(const std::vector<TVertex> &p_vertices) const -> gpu::VertexBufferHandle
		{
			const uint64 vbo_size{sizeof(TVertex) * p_vertices.size()};
			return createGPURef<gpu::VKVertexBuffer>(static_cast<const void *>(p_vertices.data()), vbo_size);
		}

		template<typename TIndex>
		[[nodiscard]] auto createIndexBuffer(const std::vector<TIndex> &p_indices) const -> gpu::IndexBufferHandle
		{
			const uint64 ibo_size{sizeof(TIndex) * p_indices.size()};
			return createGPURef<gpu::VKIndexBuffer>(static_cast<const void *>(p_indices.data()), ibo_size);
		}

		template<typename TUBOStruct>
		[[nodiscard]] auto createUniformBuffer() const -> gpu::UniformBufferHandle
		{
			constexpr uint64 ubo_size{sizeof(TUBOStruct)};
			return createGPURef<gpu::VKUniformBuffer>(ubo_size);
		}

		template<typename TUBOStruct>
		[[nodiscard]] auto createUniformBuffers(uint32 p_count) const -> gpu::UniformBufferPFFHandle
		{
			constexpr uint64 ubo_size{sizeof(TUBOStruct)};
			return createGPURef<gpu::VKUniformBufferPFF>(ubo_size, p_count);
		}

		auto getSampler(ESamplerType p_type) const -> gpu::DescriptorSlot;

		[[nodiscard]] auto createImageRef(const io::filesystem::Path &p_path) -> RefPtr<Image>;
		[[nodiscard]] auto createImageUnique(const io::filesystem::Path &p_path) -> UniquePtr<Image>;

		[[nodiscard]] auto loadTextureIntoImage(const io::filesystem::Path &p_path) const -> gpu::RawImageHandle;

		[[nodiscard]] auto createAttachmentImageRaw(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
													vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageHandle;
		[[nodiscard]] auto createMultisampleAttachmentImage(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
															vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageHandle;

		[[nodiscard]] auto createAttachmentImage(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
												 vk::Format p_format = vk::Format::eUndefined) -> RefPtr<Image>;
		[[nodiscard]] auto createAttachmentTexture(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
												   vk::Format p_format = vk::Format::eUndefined) const -> gpu::Texture2DHandle;

		[[nodiscard]] auto createEnvironmentMap(const io::filesystem::Path &p_path) -> gpu::Texture3DHandle;
		[[nodiscard]] auto createEnvironmentMap(const gpu::TextureSpecInfo &p_spec_info, const Buffer &p_data) -> gpu::Texture3DHandle;
		[[nodiscard]] auto createDiffuseIrradianceMap(const gpu::Texture3DHandle &p_environment_map) -> gpu::Texture3DHandle;

		[[nodiscard]] auto createShader(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderStage p_next_stage = EShaderStage::eNone,
										EShaderLanguage             p_shader_lang = EShaderLanguage::eHLSL) const -> gpu::DynamicShaderHandle;

		#pragma region render logic
		// For all of these, if the frame index or command buffer parameter is null / 0, they will be obtained from the swapchain instead

		auto beginRendering(const RenderingInfo &p_rendering_info, RenderPass *p_render_pass = nullptr, gpu::CommandBuffer *p_command_buffer = nullptr,
							uint32               p_frame_index                               = UINT32_MAX) const -> void;
		auto endRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer = nullptr) const -> void;

		auto beginCompute(ComputePass *p_compute_pass, gpu::CommandBuffer *p_command_buffer = nullptr, uint32 p_frame_index = UINT32_MAX) const -> void;
		auto dispatchCompute(const ComputePass *p_compute_pass, Material *p_material, const tsm::uint3 &p_work_groups, gpu::CommandBuffer *p_command_buffer = nullptr,
							 uint32             p_frame_index = UINT32_MAX) const -> void;

		auto XM_CALLCONV renderGeometry(gpu::Pipeline *p_pipeline, gpu::VertexBuffer *p_vertex_buffer, gpu::IndexBuffer *p_index_buffer, uint32 p_index_count,
										Material *     p_material, Dx::FXMMATRIX      p_transform, gpu::CommandBuffer *  p_command_buffer = nullptr,
										uint32         p_frame_index                                                                      = UINT32_MAX) const -> void;

		auto renderFullscreenQuad(const RenderPass *p_render_pass, Material *p_material, gpu::CommandBuffer *p_command_buffer = nullptr,
								  uint32            p_frame_index                                                             = UINT32_MAX) const -> void;

		auto XM_CALLCONV renderMesh(const MeshData *    p_mesh, uint32                     p_submesh_index, gpu::Pipeline *p_pipeline, Dx::FXMMATRIX p_transform,
									gpu::CommandBuffer *p_command_buffer = nullptr, uint32 p_frame_index = UINT32_MAX) const -> void;

		auto XM_CALLCONV renderMesh(const MeshData *p_mesh, uint32 p_submesh_index, gpu::Pipeline *p_pipeline, Dx::FXMMATRIX p_transform, Material *p_override_material,
									gpu::CommandBuffer *p_command_buffer = nullptr, uint32 p_frame_index = UINT32_MAX) const -> void;
		#pragma endregion

		#pragma region new render logic


		#pragma endregion

	private:
		RenderContextSpecInfo m_specInfo{};

		OwningPtr<gpu::VKInstance>       m_backendInstance{nullptr};
		OwningPtr<gpu::VKPhysicalDevice> m_physicalDevice{nullptr};
		OwningPtr<gpu::VKLogicalDevice>  m_logicalDevice{nullptr};

		OwningPtr<gpu::VKDescriptorHeap>                      m_descriptorHeap{nullptr};
		std::unordered_map<ESamplerType, gpu::DescriptorSlot> m_samplers;

		UniquePtr<ShaderCompiler> m_shaderCompiler{nullptr};

		OwningPtr<Globals> m_globals{nullptr};

		// Doing this means that we don't have to pass the current command buffer into every function, making it easier to use for the client API
		NonOwningPtr<gpu::VKCommandBuffer> m_currentSwapchainCommandBuffer{nullptr};
	};
}
