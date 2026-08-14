#pragma once

#include "render_attachment.hpp"
#include "shader_compiler.hpp"
#include "toast_gpu/buffer_layout.hpp"
#include "toast_lib/ptr.hpp"

#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_descriptor_heap.hpp"
#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_image.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster::render
{
	class Globals;
	class ShaderCompiler;
	class DynamicMaterial;

	enum class ESamplerType
	{
		eDefault,
		eNearest,
		eIrradianceMap,
		eBRDFLUT
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

		TST_PUSH_CONSTANT_BLOCK(MeshPushConstants)
		{
			Dx::XMFLOAT4X4 model;

			tsm::float4 albedoColour;

			uint32 samplerIndex;

			uint32 albedoMap;
			uint32 normalMap;
			bool32 hasNormalMap;

			float32 roughness;
			float32 metalness;

			char _padd[8];
		};

		RenderContext(const RenderContextSpecInfo &p_spec_info);
		~RenderContext();

		[[nodiscard]] auto getBackendInstance() const -> gpu::VKInstance *;
		[[nodiscard]] auto getPhysicalDevice() const -> gpu::VKPhysicalDevice *;
		[[nodiscard]] auto getLogicalDevice() const -> gpu::VKLogicalDevice *;

		[[nodiscard]] auto getDescriptorHeap() const -> gpu::VKDescriptorHeap *;

		[[nodiscard]] auto getGPUContext() const -> gpu::VKGPUContext *;

		[[nodiscard]] auto getGlobals() const -> const Globals *;

		#pragma region gpu operations
		auto gpuWaitIdle() const -> void;
		#pragma endregion

		[[nodiscard]] auto getCurrentFrameIndex() const -> uint32;
		auto               setCurrentFrameIndex(uint32 p_index) -> void;
		auto               performGarbageCollection() const -> void;

		[[nodiscard]] auto getCurrentCommandBuffer() const -> gpu::VKCommandBuffer *;
		auto               setCurrentCommandBuffer(gpu::VKCommandBuffer *p_cmd) -> void; // ONLY THE APPLICATION SHOULD USE TS...

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createRef(TArgs &&... p_args) -> RefPtr<TObj>
		{
			return makeReference<TObj>(*this, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createUnique(TArgs &&... p_args) -> UniquePtr<TObj>
		{
			return makeUnique<TObj>(*this, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the logical device into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPURef(TArgs &&... p_args) const -> RefPtr<TObj>
		{
			return makeReference<TObj>(*m_gpuCtx, std::forward<TArgs>(p_args)...);
		}

		// Use for objects that take the render context into their constructor
		template<typename TObj, typename... TArgs>
		[[nodiscard]] auto createGPUUnique(TArgs &&... p_args) const -> UniquePtr<TObj>
		{
			return makeUnique<TObj>(*m_gpuCtx, std::forward<TArgs>(p_args)...);
		}

		auto getSampler(ESamplerType p_type) const -> gpu::DescriptorSlot;

		[[nodiscard]] auto createImageRef(const io::filesystem::Path &p_path) -> gpu::ImageHandle;
		[[nodiscard]] auto createImageUnique(const io::filesystem::Path &p_path) -> gpu::ImageUnique;

		[[nodiscard]] auto loadTextureIntoImage(const io::filesystem::Path &p_path) const -> gpu::RawImageHandle;

		[[nodiscard]] auto createAttachmentImageRaw(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
													vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageHandle;
		[[nodiscard]] auto createMultisampleAttachmentImage(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
															vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageHandle;
		[[nodiscard]] auto createMultisampleAttachmentImageUnique(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
																  vk::Format p_format = vk::Format::eUndefined) const -> gpu::RawImageUnique;

		[[nodiscard]] auto createAttachmentImage(tsm::uint2 p_size, vk::ImageAspectFlags p_image_aspect_flags,
												 vk::Format p_format = vk::Format::eUndefined) -> gpu::ImageHandle;

		[[nodiscard]] auto createEnvironmentMapImage(const io::filesystem::Path &p_path) -> gpu::ImageHandle;
		[[nodiscard]] auto createDiffuseIrradianceMapImage(const gpu::ImageHandle &p_environment_map) -> gpu::ImageHandle;
		[[nodiscard]] auto createSpecularIrradianceMapImage(const gpu::ImageHandle &p_environment_map) -> gpu::ImageHandle;

		[[nodiscard]] auto createShader(const io::filesystem::Path &p_path, EShaderStage p_stage, EShaderStage p_next_stage = EShaderStage::eNone,
										EShaderLanguage             p_shader_lang = EShaderLanguage::eHLSL) const -> gpu::ShaderHandle;

		[[nodiscard]] auto createShaderFromSpirV(const io::filesystem::Path &p_spir_v_path, EShaderStage p_stage,
												 EShaderStage                p_next_stage = EShaderStage::eNone) const -> gpu::ShaderHandle;

		#pragma region new render logic

		auto beginRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer = nullptr) const -> void;
		auto endRendering(const RenderingInfo &p_rendering_info, gpu::CommandBuffer *p_command_buffer = nullptr) const -> void;

		auto renderFullscreenQuad(gpu::CommandBuffer *p_command_buffer = nullptr) const -> void;
		auto renderFullscreenQuadMeshShader(gpu::CommandBuffer *p_command_buffer = nullptr) const -> void;

		#pragma endregion

	private:
		RenderContextSpecInfo m_specInfo{};

		UniquePtr<gpu::VKGPUContext> m_gpuCtx{nullptr};

		std::unordered_map<ESamplerType, gpu::DescriptorSlot> m_samplers;

		UniquePtr<ShaderCompiler> m_shaderCompiler{nullptr};

		OwningPtr<Globals> m_globals{nullptr};
	};
}
