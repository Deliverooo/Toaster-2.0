#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "scene_renderer.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"

namespace toaster
{
	SceneRenderer::SceneRenderer(gpu::VKLogicalDevice *p_device, const SceneRendererSpecInfo &p_spec_info) : m_device(p_device), m_specInfo(p_spec_info)
	{
		TST_ASSERT_MSG(m_specInfo.scene, "This is called SceneRenderer, please provide a scene!");
		{
			constexpr vk::DeviceSize ubo_size{sizeof(CameraUB)};
			m_cameraUBOs       = m_device->alloc<gpu::VKUniformBufferPFF>(ubo_size, m_device->getSpecInfo().maxFramesInFlight);
			m_mappedCameraUBOs = m_cameraUBOs->mapAllMemory(ubo_size, 0);
		}
		{
			constexpr vk::DeviceSize ubo_size{sizeof(DirectionalLightUB)};
			m_directionalLightUBOs       = m_device->alloc<gpu::VKUniformBufferPFF>(ubo_size, m_device->getSpecInfo().maxFramesInFlight);
			m_mappedDirectionalLightUBOs = m_directionalLightUBOs->mapAllMemory(ubo_size, 0);
		}
		{
			constexpr vk::DeviceSize ubo_size{sizeof(PointLightUB)};
			m_pointLightUBOs       = m_device->alloc<gpu::VKUniformBufferPFF>(ubo_size, m_device->getSpecInfo().maxFramesInFlight);
			m_mappedPointLightUBOs = m_pointLightUBOs->mapAllMemory(ubo_size, 0);
		}
		{
			constexpr vk::DeviceSize ubo_size{sizeof(SceneDataUB)};
			m_sceneDataUBOs       = m_device->alloc<gpu::VKUniformBufferPFF>(ubo_size, m_device->getSpecInfo().maxFramesInFlight);
			m_mappedSceneDataUBOs = m_sceneDataUBOs->mapAllMemory(ubo_size, 0);
		}

		{
			constexpr vk::DeviceSize ssbo_size{sizeof(int32)};
			m_computeStorageBuffers = m_device->alloc<gpu::VKStorageBufferPFF>(ssbo_size, m_device->getSpecInfo().maxFramesInFlight);
		}

		m_skyboxTexture = m_device->alloc<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, m_specInfo.resourceDirectory / "environments/'Environment_map'.jpg");

		#pragma region depth-pre
		{
			gpu::PipelineCreateInfo depth_pre_pipeline_create_info{};
			depth_pre_pipeline_create_info.vertexBufferLayout = {
				{gpu::EBufferDataType::eFloat3, "a_Position"},
				{gpu::EBufferDataType::eFloat3, "a_Normal"},
				{gpu::EBufferDataType::eFloat3, "a_Tangent"},
				{gpu::EBufferDataType::eFloat3, "a_Bitangent"},
				{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
			};
			depth_pre_pipeline_create_info.depthFormat = vk::Format::eD32Sfloat;
			depth_pre_pipeline_create_info.shader      = Globals::getShaderLibrary().get("Depth-Pre");
			m_depthPrePipeline                         = m_device->alloc<gpu::VKPipeline>(depth_pre_pipeline_create_info);

			m_depthPrePass = m_device->alloc<gpu::VKRenderPass>(m_depthPrePipeline);
			m_depthPrePass->setInput("Camera", m_cameraUBOs);

			m_depthPrePass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...

			gpu::TextureSpecInfo depth_pre_attachment_texture_spec_info{};
			depth_pre_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			depth_pre_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			depth_pre_attachment_texture_spec_info.format = vk::Format::eD32Sfloat;
			m_depthPreAttachmentTexture                   = m_device->alloc<gpu::VKTexture2D>(depth_pre_attachment_texture_spec_info);
		}
		#pragma endregion

		#pragma region light culling
		{
			gpu::VKShader::Bytecode cs_bytecode{io::filesystem::readBinary( m_specInfo.resourceDirectory / "../bin/shaders/test.comp.glsl.spv")};
			TST_ASSERT_MSG(!cs_bytecode.empty(), "Failed to read shader file. Did you add it to the CMake compilation");
			gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eCompute, cs_bytecode}};
			m_lightCullingShader = m_device->alloc<gpu::VKShader>(shader_bytecode_map, "Compute-Test");

			m_lightCullingPipeline = m_device->alloc<gpu::VKComputePipeline>(m_lightCullingShader);

			m_lightCullingPass = m_device->alloc<gpu::VKComputePass>(m_lightCullingPipeline);
			m_lightCullingPass->setInput("Test", m_computeStorageBuffers);
			m_lightCullingPass->bake();

			m_lightCullingMaterial = m_device->alloc<gpu::VKMaterial>(m_lightCullingShader);
		}
		#pragma endregion

		#pragma region skybox
		{
			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {{gpu::EBufferDataType::eFloat3, "a_Position"}, {gpu::EBufferDataType::eFloat2, "a_TexCoord"}};
			pipeline_create_info.colourAttachments  = {vk::Format::eR8G8B8A8Srgb};
			pipeline_create_info.shader             = Globals::getShaderLibrary().get("Skybox");
			pipeline_create_info.polygonMode        = vk::PolygonMode::eFill;
			pipeline_create_info.multisample        = false;
			m_skyboxPipeline                        = m_device->alloc<gpu::VKPipeline>(pipeline_create_info);

			m_skyboxPass = m_device->alloc<gpu::VKRenderPass>(m_skyboxPipeline);
			m_skyboxPass->setInput("Camera", m_cameraUBOs);

			m_skyboxPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...

			m_skyboxMaterial = m_device->alloc<gpu::VKMaterial>(Globals::getShaderLibrary().get("Skybox"));
		}
		#pragma endregion

		#pragma region geometry
		{
			gpu::PipelineCreateInfo pipeline_create_info{};
			pipeline_create_info.vertexBufferLayout = {
				{gpu::EBufferDataType::eFloat3, "a_Position"},
				{gpu::EBufferDataType::eFloat3, "a_Normal"},
				{gpu::EBufferDataType::eFloat3, "a_Tangent"},
				{gpu::EBufferDataType::eFloat3, "a_Bitangent"},
				{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
			};
			pipeline_create_info.colourAttachments = {
				vk::Format::eR8G8B8A8Srgb,
				vk::Format::eR8G8B8A8Srgb /*Positions*/,
				vk::Format::eR8G8B8A8Srgb /*Normals*/
			};
			pipeline_create_info.depthFormat  = {vk::Format::eD32Sfloat};
			pipeline_create_info.depthWrite   = false;
			pipeline_create_info.depthCompare = vk::CompareOp::eEqual;
			// pipeline_create_info.polygonMode = vk::PolygonMode::eLine;
			pipeline_create_info.shader = Globals::getShaderLibrary().get("Geometry");
			m_geometryPipeline          = m_device->alloc<gpu::VKPipeline>(pipeline_create_info);

			m_geometryPass = m_device->alloc<gpu::VKRenderPass>(m_geometryPipeline);
			m_geometryPass->setInput("Camera", m_cameraUBOs);
			m_geometryPass->setInput("DirectionalLightData", m_directionalLightUBOs);
			m_geometryPass->setInput("PointLightData", m_pointLightUBOs);
			m_geometryPass->setInput("SceneData", m_sceneDataUBOs);

			m_geometryPass->bake(); // TODO: rename ts to toast
			//						   Its funny because the engine is called Toaster...

			gpu::TextureSpecInfo geometry_positions_attachment_texture_spec_info{};
			geometry_positions_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			geometry_positions_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			geometry_positions_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
			m_geometryPositionsAttachmentTexture                   = m_device->alloc<gpu::VKTexture2D>(geometry_positions_attachment_texture_spec_info);

			gpu::TextureSpecInfo geometry_normals_attachment_texture_spec_info{};
			geometry_normals_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			geometry_normals_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			geometry_normals_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
			m_geometryNormalsAttachmentTexture                   = m_device->alloc<gpu::VKTexture2D>(geometry_normals_attachment_texture_spec_info);
		}
		#pragma endregion

		{
			gpu::TextureSpecInfo resolve_colour_attachment_texture_spec_info{};
			resolve_colour_attachment_texture_spec_info.width  = m_specInfo.viewportWidth;
			resolve_colour_attachment_texture_spec_info.height = m_specInfo.viewportHeight;
			resolve_colour_attachment_texture_spec_info.format = vk::Format::eR8G8B8A8Srgb;
			m_outputColourTexture                              = m_device->alloc<gpu::VKTexture2D>(resolve_colour_attachment_texture_spec_info);
		}

		Renderer2DSpecInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth   = m_specInfo.viewportWidth;
		renderer_2d_create_info.renderTargetHeight  = m_specInfo.viewportHeight;
		renderer_2d_create_info.overrideAttachments = true;
		m_renderer2D                                = make_reference<Renderer2D>(m_device, renderer_2d_create_info);
	}

	SceneRenderer::~SceneRenderer()
	{
		m_sceneDataUBOs->unmapAllMemory();
		m_directionalLightUBOs->unmapAllMemory();
		m_cameraUBOs->unmapAllMemory();
	}

	auto SceneRenderer::begin([[maybe_unused]] const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index, const glm::mat4 &p_view_matrix,
							  const glm::mat4 &                               p_projection_matrix) -> void
	{
		CameraUB camera_ub{};
		camera_ub.view       = p_view_matrix;
		camera_ub.proj       = p_projection_matrix;
		camera_ub.proj[1][1] *= -1.0f; // Silly opengl
		std::memcpy(m_mappedCameraUBOs[p_frame_index], &camera_ub, sizeof(CameraUB));

		const SceneLightEnvironment &light_environment{m_specInfo.scene->getLightEnvironment()};
		{
			DirectionalLightUB directional_light_ub{};
			directional_light_ub.count = light_environment.directionalLights.size();
			for (uint32 i{0u}; i < DirectionalLightUB::c_maxDirectionalLights && i < light_environment.directionalLights.size(); ++i)
			{
				directional_light_ub.directionalLights[i].direction = light_environment.directionalLights[i].direction;
				directional_light_ub.directionalLights[i].radiance  = light_environment.directionalLights[i].radiance;
			}
			std::memcpy(m_mappedDirectionalLightUBOs[p_frame_index], &directional_light_ub, sizeof(DirectionalLightUB));
		}
		{
			PointLightUB point_light_ub{};
			point_light_ub.count = std::min(static_cast<uint32>(light_environment.pointLights.size()), PointLightUB::c_maxPointLights);
			for (uint32 i{0u}; i < PointLightUB::c_maxPointLights && i < light_environment.pointLights.size(); ++i)
			{
				point_light_ub.pointLights[i].position = light_environment.pointLights[i].position;
				point_light_ub.pointLights[i].radiance = light_environment.pointLights[i].radiance;
				// point_light_ub.pointLights[i].radius   = light_environment.pointLights[i].radius;
				// point_light_ub.pointLights[i].falloff  = light_environment.pointLights[i].falloff;
			}
			std::memcpy(m_mappedPointLightUBOs[p_frame_index], &point_light_ub, sizeof(PointLightUB));
		}

		SceneDataUB scene_data_ub{};
		scene_data_ub.cameraPos = glm::inverse(p_view_matrix)[3];
		std::memcpy(m_mappedSceneDataUBOs[p_frame_index], &scene_data_ub, sizeof(SceneDataUB));
	}

	auto SceneRenderer::end(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		_renderDepthPrePass(p_cmd, p_frame_index);
		_renderLightCullingPass(p_cmd, p_frame_index);
		_renderSkyboxPass(p_cmd, p_frame_index);
		_renderGeometryPass(p_cmd, p_frame_index);

		m_meshDrawCommands.clear();

		// int32 data{0};
		// void *mapped{m_computeStorageBuffers->getSSBO(p_frame_index)->mapMemory(0, sizeof(int32))};
		// std::memcpy(&data, mapped, sizeof(int32));
		// m_computeStorageBuffers->getSSBO(p_frame_index)->unmapMemory();
		// LOG_INFO("{}", data);
	}

	auto SceneRenderer::renderMesh(RefPtr<gpu::VKMesh> p_mesh, const glm::mat4 &p_transform) -> void
	{
		DrawCommand &draw_command{m_meshDrawCommands.emplace_back()};
		draw_command.mesh      = p_mesh;
		draw_command.transform = p_transform;
	}

	auto SceneRenderer::getSpecInfo() const -> const SceneRendererSpecInfo &
	{
		return m_specInfo;
	}

	auto SceneRenderer::getOutputColourTexture() const -> const RefPtr<gpu::VKTexture2D> &
	{
		return m_outputColourTexture;
	}

	auto SceneRenderer::getOutputDepthTexture() const -> const RefPtr<gpu::VKTexture2D> &
	{
		return m_depthPreAttachmentTexture;
	}

	auto SceneRenderer::getGeometryPositionsTexture() const -> const RefPtr<gpu::VKTexture2D> &
	{
		return m_geometryPositionsAttachmentTexture;
	}

	auto SceneRenderer::getGeometryNormalsTexture() const -> const RefPtr<gpu::VKTexture2D> &
	{
		return m_geometryNormalsAttachmentTexture;
	}

	auto SceneRenderer::getRenderer2D() -> RefPtr<Renderer2D>
	{
		return m_renderer2D;
	}

	auto SceneRenderer::onResize(uint32 p_width, uint32 p_height) -> void
	{
		TST_ASSERT_MSG(p_width != 0 && p_height != 0, "Cannot resize to 0");

		if (m_specInfo.viewportWidth != p_width || m_specInfo.viewportHeight != p_height)
		{
			m_specInfo.viewportWidth  = p_width;
			m_specInfo.viewportHeight = p_height;

			m_depthPreAttachmentTexture->resize(p_width, p_height);

			m_geometryPositionsAttachmentTexture->resize(p_width, p_height);
			m_geometryNormalsAttachmentTexture->resize(p_width, p_height);
			m_outputColourTexture->resize(p_width, p_height);

			m_renderer2D->onResize(p_width, p_height);
		}
	}

	auto SceneRenderer::setEnvironmentBackground(const RefPtr<gpu::VKTexture2D> &p_texture) -> void
	{
		m_skyboxTexture = p_texture;
	}

	auto SceneRenderer::_renderDepthPrePass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.image      = m_depthPreAttachmentTexture->getImage();
		depth_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;
		rendering_info.pDepthAttachment  = &depth_attachment_info;

		render::beginRendering(rendering_info, p_cmd, p_frame_index, m_depthPrePass);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				render::renderMesh(p_cmd, p_frame_index, draw_cmd.mesh, i, m_depthPrePipeline, draw_cmd.transform * draw_cmd.mesh->getSubmeshes()[i].localTransform,
								   nullptr);
			}
		}

		render::endRendering(rendering_info, p_cmd);
	}

	auto SceneRenderer::_renderLightCullingPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		render::beginCompute(p_cmd, p_frame_index, m_lightCullingPass);

		render::dispatchCompute(p_cmd, p_frame_index, m_lightCullingPass, m_lightCullingMaterial, 1, 1, 1);

		render::endCompute(p_cmd, p_frame_index, m_lightCullingPass);
	}

	auto SceneRenderer::_renderSkyboxPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		m_skyboxMaterial->set("u_Texture", m_skyboxTexture);

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.clearValue = vk::ClearColorValue{1.0f, 0.0f, 0.0f, 1.0f};
		colour_attachment_info.image      = m_outputColourTexture->getImage();
		colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

		render::beginRendering(rendering_info, p_cmd, p_frame_index, m_skyboxPass);
		render::renderFullscreenQuad(p_cmd, p_frame_index, m_skyboxPipeline, m_skyboxMaterial);
		render::endRendering(rendering_info, p_cmd);
	}

	auto SceneRenderer::_renderGeometryPass(const vk::raii::CommandBuffer &p_cmd, uint32 p_frame_index) -> void
	{
		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{m_specInfo.viewportOffsetX, m_specInfo.viewportOffsetY}, {m_specInfo.viewportWidth, m_specInfo.viewportHeight}};
		rendering_info.layerCount = 1;

		gpu::RenderingAttachmentInfo &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
		colour_attachment_info.image      = m_outputColourTexture->getImage();
		colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eNone;
		colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

		gpu::RenderingAttachmentInfo &positions_attachment_info{rendering_info.colourAttachments.emplace_back()};
		positions_attachment_info.clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
		positions_attachment_info.image      = m_geometryPositionsAttachmentTexture->getImage();
		positions_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
		positions_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

		gpu::RenderingAttachmentInfo &normals_attachment_info{rendering_info.colourAttachments.emplace_back()};
		normals_attachment_info.clearValue = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
		normals_attachment_info.image      = m_geometryNormalsAttachmentTexture->getImage();
		normals_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
		normals_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;

		gpu::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue = vk::ClearDepthStencilValue{1.0f, 0u};
		depth_attachment_info.image      = m_depthPreAttachmentTexture->getImage();
		depth_attachment_info.loadOp     = vk::AttachmentLoadOp::eLoad;
		depth_attachment_info.storeOp    = vk::AttachmentStoreOp::eDontCare;
		rendering_info.pDepthAttachment  = &depth_attachment_info;

		render::beginRendering(rendering_info, p_cmd, p_frame_index, m_geometryPass);

		for (const auto &draw_cmd: m_meshDrawCommands)
		{
			for (uint32 i{0u}; i < draw_cmd.mesh->getSubmeshes().size(); ++i)
			{
				render::renderMesh(p_cmd, p_frame_index, draw_cmd.mesh, i, m_geometryPipeline, draw_cmd.transform * draw_cmd.mesh->getSubmeshes()[i].localTransform);
			}
		}

		render::endRendering(rendering_info, p_cmd);
	}
}
