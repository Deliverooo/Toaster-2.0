#include "editor_layer.hpp"

#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"
#include "toast_lib/events/key_event.hpp"
#include "toast_render/globals.hpp"
#include "toast_render/renderer.hpp"
#include "toast_scene/components.hpp"
#include "toast_scene/entity.hpp"
#include "toast_scene/scene_renderer.hpp"

#include "toast_asset/asset_manager.hpp"

#include <imgui.h>
namespace ig = ImGui;

#include "glm/gtc/type_ptr.hpp"
#include "toast_kernel/input.hpp"

#include <ImGuizmo.h>
namespace igz = ImGuizmo;

#include "backends/imgui_impl_vulkan.h"
#include "toast_lib/math/math_matrix.hpp"
#include "toast_lib/os/file_dialog.hpp"

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_editorCamera(90.0f, 1.777f, 0.1f, 100.0f)
	{
	}

	auto EditorLayer::onInit() -> void
	{
		const auto &app{getApp()};
		m_device = app.getWindow().getLogicalDevice();
		const auto swapchain{app.getWindow().getSwapchain()};

		m_windowWidth  = std::max(swapchain->getExtent().width, 1u);
		m_windowHeight = std::max(swapchain->getExtent().height, 1u);

		m_viewportWidth  = m_windowWidth;
		m_viewportHeight = m_windowHeight;

		m_editorCamera.setViewportSize(static_cast<float32>(m_viewportWidth), static_cast<float32>(m_viewportHeight));

		swapchain->setBeginFrameCallback([](gpu::VKLogicalDevice *device, const uint32 frame_index) -> void
		{
			device->setCurrentFrameIndex(frame_index);
			device->performGarbageCollection();
		});

		swapchain->setResizeCallback([this](const uint32 width, const uint32 height) -> void
		{
			m_windowWidth  = width;
			m_windowHeight = height;

			// m_sceneRenderer->onResize(width, height);
		});

		auto fullscreen_shader{Globals::getShaderLibrary().get("Composite")};

		gpu::PipelineCreateInfo fullscreen_pipeline_create_info{};
		fullscreen_pipeline_create_info.colourAttachments = {swapchain->getSurfaceFormat().format};
		// fullscreen_pipeline_create_info.depthFormat        = swapchain->getDepthFormat();
		fullscreen_pipeline_create_info.shader             = fullscreen_shader;
		fullscreen_pipeline_create_info.cullMode           = vk::CullModeFlagBits::eNone; // We don't want to cull our viewport
		fullscreen_pipeline_create_info.vertexBufferLayout = gpu::BufferLayout{
			{gpu::EBufferDataType::eFloat3, "a_Position"},
			{gpu::EBufferDataType::eFloat2, "a_TexCoord"}
		};
		m_fullscreenPipeline = m_device->alloc<gpu::VKPipeline>(fullscreen_pipeline_create_info);
		m_fullscreenPass     = m_device->alloc<gpu::VKRenderPass>(m_fullscreenPipeline);
		m_fullscreenPass->bake();

		m_fullscreenMaterial = m_device->alloc<gpu::VKMaterial>(fullscreen_shader);

		gpu::ImageCreateInfo fullscreen_attachment_create_info{};
		fullscreen_attachment_create_info.width  = m_viewportWidth;
		fullscreen_attachment_create_info.height = m_viewportHeight;
		fullscreen_attachment_create_info.usage  = vk::ImageUsageFlagBits::eColorAttachment;
		fullscreen_attachment_create_info.format = vk::Format::eR8G8B8A8Srgb;
		m_fullscreenAttachmentImage              = m_device->alloc<gpu::VKImage2D>(fullscreen_attachment_create_info);

		m_scene = make_reference<Scene>(m_device, "Main Scene");

		m_sceneHierarchyPanel = make_unique<SceneHierarchyPanel>(m_device, m_scene);

		SceneRendererSpecInfo scene_renderer_spec_info{};
		scene_renderer_spec_info.viewportWidth  = m_viewportWidth;
		scene_renderer_spec_info.viewportHeight = m_viewportHeight;
		scene_renderer_spec_info.scene          = m_scene;
		m_sceneRenderer                         = make_reference<SceneRenderer>(m_device, scene_renderer_spec_info);

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = m_viewportWidth;
		renderer_2d_create_info.renderTargetHeight = m_viewportHeight;
		m_renderer2D                               = make_reference<Renderer2D>(m_device, renderer_2d_create_info);

		{
			Entity orbo_entity{m_scene->createEntity()};
			auto & transform_comp{orbo_entity.getComponent<TransformComponent>()};
			transform_comp.translation = {0.0f, 0.0f, 0.0f};
			transform_comp.scale       = {1.0f, 1.0f, 1.0f};
			auto &mc{orbo_entity.addComponent<MeshComponent>()};
			mc.mesh = m_device->alloc<gpu::VKMesh>("../resources/meshes/Orbo.fbx", Globals::getShaderLibrary().get("Geometry"));
		}
		{
			Entity point_light_entity{m_scene->createEntity()};
			auto & transform_comp{point_light_entity.getComponent<TransformComponent>()};
			transform_comp.translation = {0.0f, 0.5f, -1.0f};
			auto &src{point_light_entity.addComponent<SpriteRendererComponent>()};
			src.colour = tsm::colours::red;
			auto &plc{point_light_entity.addComponent<PointLightComponent>()};
			plc.radiance = xyz(tsm::colours::green);
		}
	}

	auto EditorLayer::onDestroy() -> void
	{
	}

	auto EditorLayer::onUpdate(const float32 p_dt) -> void
	{
		m_time += p_dt;

		if (m_canOperateCamera)
			m_editorCamera.onUpdate(p_dt);

		const auto &app{getApp()};
		const auto  swapchain{app.getWindow().getSwapchain()};

		const auto & cmd_buf{swapchain->getCurrentCommandBuffer()};
		const uint32 frame_index{swapchain->getFrameIndex()};

		m_scene->onUpdate(p_dt);
		m_scene->onRender(cmd_buf, frame_index, p_dt, m_sceneRenderer, m_editorCamera.getViewMatrix(), m_editorCamera.getProjectionMatrix());

		m_fullscreenMaterial->set("u_Texture", m_sceneRenderer->getOutputColourTexture());

		gpu::RenderingInfo rendering_info{};
		rendering_info.renderArea = vk::Rect2D{{0, 0}, {m_viewportWidth, m_viewportHeight}};

		auto &colour_attachment_info{rendering_info.colourAttachments.emplace_back()};
		colour_attachment_info.image      = m_fullscreenAttachmentImage;
		colour_attachment_info.loadOp     = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp    = vk::AttachmentStoreOp::eStore;
		colour_attachment_info.clearValue = vk::ClearColorValue{1.0f, 1.0f, 1.0f, 1.0f};

		Renderer::beginRendering(rendering_info, cmd_buf, frame_index, m_fullscreenPass);
		Renderer::renderFullscreenQuad(cmd_buf, frame_index, m_fullscreenPipeline, m_fullscreenMaterial);
		Renderer::endRendering(rendering_info, cmd_buf);
	}

	auto EditorLayer::onEvent(Event &p_event) -> void
	{
		EventDispatcher event_dispatcher{p_event};
		event_dispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(EditorLayer::_onKeyPressEvent));
		event_dispatcher.dispatch<WindowFileDropEvent>(TST_BIND_EVENT_FN(EditorLayer::_onWindowFileDropEvent));

		if (m_canOperateCamera)
			m_editorCamera.onEvent(p_event);
	}

	auto EditorLayer::onUIRender() -> void
	{
		const auto &app{getApp()};
		const auto  swapchain{app.getWindow().getSwapchain()};
		uint32      frame_index{swapchain->getFrameIndex()};

		m_sceneHierarchyPanel->onUIRender(frame_index);

		ig::Begin("Viewport");
		ImVec2 window_size{ig::GetWindowSize()};

		if (m_viewportWidth != window_size.x || m_viewportHeight != window_size.y)
		{
			LOG_INFO("Resizing viewport");
			m_device->submitResourceUpdate([this, window_size]() -> void
			{
				m_viewportWidth  = window_size.x;
				m_viewportHeight = window_size.y;

				m_fullscreenAttachmentImage->resize(static_cast<uint32>(window_size.x), static_cast<uint32>(window_size.y));

				m_sceneRenderer->onResize(static_cast<uint32>(window_size.x), static_cast<uint32>(window_size.y));
				m_editorCamera.setViewportSize(window_size.x, window_size.y);
			});
		}
		if (ig::IsWindowFocused() || ig::IsWindowHovered())
			m_canOperateCamera = false;
		else
			m_canOperateCamera = true;

		// if (m_imguiSceneRendererDescriptorSets[frame_index])
		ig::Image((VkDescriptorSet) m_fullscreenMaterial->getDescriptorSet(frame_index), window_size);
		Entity selected_entity = m_sceneHierarchyPanel->getSelectedEntity();
		if (selected_entity && m_gizmoType != -1)
		{
			auto w = ig::GetWindowWidth();
			auto h = ig::GetWindowHeight();

			igz::SetOrthographic(false);
			igz::SetDrawlist(ig::GetForegroundDrawList());         // Draw to the main surface
			igz::SetRect(0, 0, m_viewportWidth, m_viewportHeight); // Full window area

			bool snap_transform = input::isKeyDown(input::EKeyCode::eLeftControl);

			auto &    tc               = selected_entity.getComponent<TransformComponent>();
			glm::mat4 entity_transform = tc.getTransform();
			float32   snap_value{0.5f};
			if (m_gizmoType == igz::OPERATION::ROTATE)
				snap_value = 45.0f;
			const float32 snap_values[3] = {snap_value, snap_value, snap_value};

			igz::Manipulate(glm::value_ptr(m_editorCamera.getViewMatrix()), glm::value_ptr(m_editorCamera.getProjectionMatrix()),
							static_cast<igz::OPERATION>(m_gizmoType), static_cast<igz::MODE>(m_gizmoMode), glm::value_ptr(entity_transform), nullptr,
							snap_transform ? snap_values : nullptr);
			if (igz::IsUsing())
			{
				glm::vec3 translation;
				glm::quat rotation;
				glm::vec3 scale;

				tsm::decomposeTransform(entity_transform, translation, rotation, scale);

				const glm::vec3 delta_rotation = glm::eulerAngles(rotation) - tc.rotation;
				tc.translation                 = translation;
				tc.rotation                    += delta_rotation;
				tc.scale                       = scale;
			}
		}
		ig::End();

		ig::Begin("Renderer settings");
		ig::Text("Scene renderer background");
		if (ig::Button("File", ImVec2{ig::GetContentRegionAvail().x, 0}))
		{
			auto path = os::openFileDialog({{"Texture", "png,jpg,bmp"}});
			if (io::filesystem::exists(path))
			{
				LOG_INFO("{}", path.string());

				gpu::TextureSpecInfo texture_spec_info{};
				m_sceneRenderer->setEnvironmentBackground(m_device->alloc<gpu::VKTexture2D>(texture_spec_info, path));
			}
		}

		ig::End();

		#if 1
		if (ig::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ig::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			m_canOperateCamera = false;
		else
			m_canOperateCamera = true;
		#endif
	}

	auto EditorLayer::_onWindowFileDropEvent(WindowFileDropEvent &p_event) -> bool
	{
		LOG_INFO("{}", p_event.toStr());

		for (const auto &path: p_event.getFilepaths())
		{
			if (path.ends_with(".fbx") || path.ends_with(".obj") || path.ends_with(".glb") || path.ends_with(".gltf"))
			{
				Entity e{m_scene->createEntity(io::filesystem::Path{path}.stem().string())};
				auto & mc{e.addComponent<MeshComponent>()};
				mc.mesh = m_device->alloc<gpu::VKMesh>(path, Globals::getShaderLibrary().get("Geometry"));
			}
			if (path.ends_with(".png") || path.ends_with(".jpg") || path.ends_with(".jpeg") || path.ends_with(".bmp"))
			{
				Entity e{m_scene->createEntity(io::filesystem::Path{path}.stem().string())};
				auto & src{e.addComponent<SpriteRendererComponent>()};
				src.texture = m_device->alloc<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, path);
			}
		}
		return false;
	}

	auto EditorLayer::_onKeyPressEvent(KeyPressEvent &p_event) -> bool
	{
		#if 1
		if (!m_canOperateCamera)
		{
			switch (p_event.getKeyCode())
			{
				case input::EKeyCode::eQ:
					m_gizmoType = -1;
					break;
				case input::EKeyCode::eW:
					m_gizmoType = igz::OPERATION::TRANSLATE;
					break;
				case input::EKeyCode::eE:
					m_gizmoType = igz::OPERATION::ROTATE;
					break;
				case input::EKeyCode::eR:
					m_gizmoType = igz::OPERATION::SCALE;
					break;
				case input::EKeyCode::eL:
				{
					// Switch between world and local space transforming for the gizmos
					if (input::isKeyDown(input::EKeyCode::eLeftAlt))
					{
						if (m_gizmoMode == igz::MODE::LOCAL)
							m_gizmoMode = igz::MODE::WORLD;
						else
							m_gizmoMode = igz::MODE::LOCAL;
					}
					break;
				}
				default: break;
			}
		}
		#endif
		if (p_event.getKeyCode() == input::EKeyCode::eEscape)
			getApp().close();
		return false;
	}
}
