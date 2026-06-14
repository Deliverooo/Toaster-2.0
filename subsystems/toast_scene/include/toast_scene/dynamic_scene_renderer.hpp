#pragma once

#include "scene.hpp"
#include "toast_gpu/vk/vk_storage_image.hpp"
#include "toast_gpu/vk/vk_uniform_buffer.hpp"
#include "toast_render/compute_pass.hpp"
#include "toast_render/mesh.hpp"

namespace toaster
{
	class TST_SCENE_API DynamicSceneRenderer
	{
	public:
		DynamicSceneRenderer(Scene *p_scene, tsm::uint2 p_viewport_size);
		~DynamicSceneRenderer();

		auto             onRender() -> void;
		auto XM_CALLCONV onRender(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;

		// Passing the position avoids unnecessary calculations
		auto XM_CALLCONV begin(Dx::FXMVECTOR p_camera_position, Dx::FXMMATRIX p_view_matrix, Dx::CXMMATRIX p_projection_matrix) -> void;
		auto             end() -> void;
		auto XM_CALLCONV renderMesh(const render::DynamicMeshHandle &p_mesh, Dx::FXMMATRIX p_transform) -> void;

		auto getViewportSize() const -> tsm::uint2 { return m_viewportSize; }

		auto getMSAAColourImage() -> gpu::RawImageHandle & { return m_MSAAColourImage; }
		auto getMSAADepthImage() -> gpu::RawImageHandle & { return m_MSAADepthImage; }
		auto getMSAAGeometryNormalsImage() -> gpu::RawImageHandle & { return m_MSAAGeometryNormalsImage; }
		auto getMSAAGeometryPositionsImage() -> gpu::RawImageHandle & { return m_MSAAGeometryPositionsImage; }

		auto getColourImage() const -> const render::ImageHandle & { return m_colourImage; }
		auto getDepthImage() const -> const render::ImageHandle & { return m_depthImage; }
		auto getGeometryNormalsImage() const -> const render::ImageHandle & { return m_geometryNormalsImage; }
		auto getGeometryPositionsImage() const -> const render::ImageHandle & { return m_geometryPositionsImage; }

		// auto getSSAONoiseTexture() const -> const render::ImageHandle & { return m_SSAONoiseTexture; }
		// auto getSSAOTexture() const -> const render::ImageHandle & { return m_SSAOTexture; }
		// auto getSSAOBlurredImage() const -> const render::ImageHandle & { return m_SSAOBlurredImage; }

		// auto getRenderer2D() -> const RefPtr<render::Renderer2D> & { return m_renderer2D; }

		auto onResize(tsm::uint2 p_size) -> void;

		auto reloadEnvironmentMaps(const render::ImageHandle &p_skybox, const render::ImageHandle &p_diffuse_irradiance) -> void;

	private:
		auto _renderDepthPrePass(gpu::VKCommandBuffer *p_cmd) -> void;
		// auto _renderAOPass(gpu::VKCommandBuffer *p_cmd) -> void;
		auto _renderSkyboxPass(gpu::VKCommandBuffer *p_cmd) -> void;
		// auto _renderGeometryPass(gpu::VKCommandBuffer *p_cmd) -> void;

		// static auto _generateSSAONoise(uint32 p_texture_size) -> std::vector<tsm::float4>;

		NonOwningPtr<Scene>                 m_scene{nullptr};
		NonOwningPtr<render::RenderContext> m_renderCtx{nullptr};

		tsm::uint2 m_viewportSize{0u};

		// RefPtr<render::Renderer2D> m_renderer2D{nullptr};
		TST_PUSH_CONSTANT_BLOCK(SceneDataConstants)
		{
			uintptr cameraPtr;            // m_cameraUBOs
			uintptr directionalLightsPtr; // m_directionalLightUBOs
			uintptr pointLightsPtr;       // m_pointLightUBOs
			uintptr sceneDataPtr;         // m_sceneDataUBOs
		};

		#pragma region depth-pre

		gpu::DynamicShaderHandle m_depthPreVertexShader{nullptr};
		gpu::DynamicShaderHandle m_depthPrePixelShader{nullptr};

		render::GraphicsStateUnique m_depthPreGraphicsState{nullptr};
		TST_PUSH_CONSTANT_BLOCK(DepthPreConstants)
		{
			uintptr cameraPtr;
			char    _padd[8];
		};

		gpu::RawImageHandle m_MSAADepthImage{nullptr};
		render::ImageHandle m_depthImage{nullptr};

		gpu::RawImageHandle m_MSAAGeometryNormalsImage{nullptr};
		render::ImageHandle m_geometryNormalsImage{nullptr};

		gpu::RawImageHandle m_MSAAGeometryPositionsImage{nullptr};
		render::ImageHandle m_geometryPositionsImage{nullptr};
		#pragma endregion

		#pragma region skybox

		gpu::DynamicShaderHandle m_skyboxVertexShader{nullptr};
		gpu::DynamicShaderHandle m_skyboxPixelShader{nullptr};

		render::GraphicsStateUnique m_skyboxGraphicsState{nullptr};
		TST_PUSH_CONSTANT_BLOCK(SkyboxConstants)
		{
			uintptr cameraPtr;
			uint32  samplerId;
			uint32  skyboxMapId;
			// char    _padd[4];
		};

		render::ImageHandle m_skyboxImage{nullptr};
		#pragma endregion

		#if 0
		#pragma region ambient occlusion

		// gpu::PipelineHandle      m_SSAOPipeline{nullptr};
		// render::RenderPassHandle m_SSAOPass{nullptr};

		// render::MaterialHandle m_SSAOFrameDataMaterial{nullptr};

		struct SSAOKernel
		{
			static constexpr uint32 c_SSAOSampleCount{64};

			Dx::XMFLOAT4 samples[c_SSAOSampleCount];

			SSAOKernel();
		}; UniquePtr<SSAOKernel>   m_SSAOKernel{nullptr}; render::ImageHandle             m_SSAONoiseTexture{nullptr}; render::ImageHandle m_SSAOTexture{nullptr};
		gpu::ComputePipelineHandle m_SSAOBlurPipeline{nullptr}; render::ComputePassHandle m_SSAOBlurPass{nullptr}; gpu::StorageImageHandle m_SSAOBlurredImage{nullptr};

		#pragma endregion

		#pragma region skybox
		gpu::PipelineHandle m_skyboxPipeline{nullptr}; render::RenderPassHandle m_skyboxPass{nullptr}; render::MaterialHandle m_skyboxMaterial{nullptr};

		#pragma endregion

		#pragma region geometry
		gpu::PipelineHandle m_geometryPipeline{nullptr}; render::RenderPassHandle m_geometryPass{nullptr};
		#pragma endregion

		#endif

		gpu::RawImageHandle m_MSAAColourImage{nullptr};
		render::ImageHandle m_colourImage{nullptr};

		struct CameraUB
		{
			Dx::XMFLOAT4X4 view;
			Dx::XMFLOAT4X4 proj;
			Dx::XMFLOAT4X4 invProj;
		};

		render::UniformBufferPFFHandle m_cameraUBOs{nullptr};

		#if 0
		struct DirectionalLightUB
		{
			static constexpr uint32 c_maxDirectionalLights{4u};

			uint32           count{0u};
			tsm::float3      _padding{0.0f};
			DirectionalLight directionalLights[c_maxDirectionalLights]{};
		}; gpu::UniformBufferPFFHandle m_directionalLightUBOs; gpu::UBOMappedDataPFF m_mappedDirectionalLightUBOs; struct PointLightUB
		{
			static constexpr uint32 c_maxPointLights{128u};

			uint32      count{0u};
			tsm::float3 _padding{0.0f};
			PointLight  pointLights[c_maxPointLights]{};
		}; gpu::UniformBufferPFFHandle m_pointLightUBOs; gpu::UBOMappedDataPFF m_mappedPointLightUBOs; struct SceneDataUB
		{
			Dx::XMFLOAT3 cameraPos{0.0f, 0.0f, 0.0f};
			char         _padd[4];
		}; gpu::UniformBufferPFFHandle m_sceneDataUBOs; gpu::UBOMappedDataPFF m_mappedSceneDataUBOs;

		#endif

		struct DrawCommand
		{
			render::DynamicMeshHandle mesh{nullptr};
			Dx::XMFLOAT4X4            transform;
		};

		std::vector<DrawCommand> m_meshDrawCommands;
	};
}
