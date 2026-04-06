#include "editor_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

#include "toaster/toast_scene/components.hpp"
#include "toaster/toast_scene/scene_serializer.hpp"

#include "editor_application.hpp"

#include "toast_lib/os/file_dialog.hpp"

#include "ui/ui_utils.hpp"

#include "toaster/toast_lib/logging.hpp"

#include <ImGuizmo.h>

#include "backends/imgui_impl_vulkan.h"
#include "glm/gtc/type_ptr.hpp"
#include "toast_gpu/vk/vk_swapchain.hpp"
namespace ig = ImGui;
namespace igz = ImGuizmo;

#include "toast_lib/math/math_matrix.hpp"

namespace toaster
{
	EditorLayer::EditorLayer(Application *p_app) : IAppLayer(p_app), m_editorCamera(90.0f, 1.7776f, 0.1f, 1000.0f)
	{
	}

	void EditorLayer::onInit()
	{
		m_scene               = make_reference<Scene>();
		m_sceneHierarchyPanel = make_reference<SceneHierarchyPanel>(m_scene);

		_createRenderTargetResources(1920u, 1080u);

		// gpu::FramebufferCreateInfo framebuffer_create_info{};
		// framebuffer_create_info.width       = 1920u;
		// framebuffer_create_info.height      = 1080u;
		// framebuffer_create_info.samples     = 1u;
		// framebuffer_create_info.attachments = {gpu::EImageFormat::eRGBA32F, gpu::EImageFormat::eRedInteger, gpu::EImageFormat::eDepth32FStencil8UInt};

		// m_framebuffer = gpu::IFramebuffer::create(framebuffer_create_info);

		// Renderer2DCreateInfo renderer_2d_create_info;
		// renderer_2d_create_info.maxQuads = 1000u;
		// m_renderer2d                     = make_reference<Renderer2D>(renderer_2d_create_info);

		auto &app            = getApp();
		m_initialWindowTitle = app.getWindow().getTitle();
		app.getWindow().setTitle(m_initialWindowTitle + " -> " + m_scene->getName());
	}

	void EditorLayer::onDestroy()
	{
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		uint32 image_index{swapchain->getImageIndex()};

		vk::Extent2D swapchain_extent{swapchain->getExtent()};

		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		if (m_viewportSize.x > 0.0f && m_viewportSize.y > 0.0f && (static_cast<float32>(m_renderTargetImageWidth) != m_viewportSize.x || static_cast<float32>(
																	   m_renderTargetImageHeight) != m_viewportSize.y))
		{
			_createRenderTargetResources(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
			m_editorCamera.setViewportSize(m_viewportSize.x, m_viewportSize.y);
			m_scene->setViewportSize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
		}

		vk::ClearValue              clear_colour = vk::ClearColorValue{0.2f, 0.3f, 0.3f, 1.0f};
		vk::RenderingAttachmentInfo colour_attachment_info{};
		colour_attachment_info.clearValue  = clear_colour;
		colour_attachment_info.imageView   = m_renderTargetImageView;
		colour_attachment_info.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colour_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		colour_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::ClearValue              clear_depth = vk::ClearDepthStencilValue{1.0f, 0u};
		vk::RenderingAttachmentInfo depth_attachment_info{};
		depth_attachment_info.clearValue  = clear_depth;
		depth_attachment_info.imageView   = m_renderTargetDepthImageView;
		depth_attachment_info.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
		depth_attachment_info.loadOp      = vk::AttachmentLoadOp::eClear;
		depth_attachment_info.storeOp     = vk::AttachmentStoreOp::eStore;

		vk::RenderingInfo rendering_info{};
		rendering_info.renderArea           = vk::Rect2D{{0, 0}, swapchain_extent};
		rendering_info.layerCount           = 1;
		rendering_info.colorAttachmentCount = 1;
		rendering_info.pColorAttachments    = &colour_attachment_info;
		rendering_info.pDepthAttachment     = &depth_attachment_info;

		vk::Viewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x        = 0.0f;
		viewport.y        = 0.0f;
		viewport.width    = static_cast<float32>(swapchain_extent.width);
		viewport.height   = static_cast<float32>(swapchain_extent.height);

		vk::Rect2D scissor{vk::Offset2D{0, 0}, swapchain_extent};

		command_buffer.beginRendering(rendering_info);
		command_buffer.setViewport(0, viewport);
		command_buffer.setScissor(0, scissor);

		if (m_viewportFocused)
			m_editorCamera.onUpdate(p_dt);

		m_scene->onUpdate(p_dt);
		// m_scene->onRender(p_dt, m_renderer2d, m_editorCamera.getViewMatrix(), m_editorCamera.getProjectionMatrix());

		auto [mx, my]      = ig::GetMousePos();
		mx                 -= m_viewportBounds[0].x;
		my                 -= m_viewportBounds[0].y;
		auto viewport_size = m_viewportBounds[1] - m_viewportBounds[0];

		my            = viewport_size.y - my;
		int32 mouse_x = static_cast<int32>(mx);
		int32 mouse_y = static_cast<int32>(my);

		if (mouse_x >= 0 && mouse_y >= 0 && mouse_x < static_cast<int32>(viewport_size.x) && mouse_y < static_cast<int32>(viewport_size.y))
		{
			// int32 pixel_data = m_framebuffer->readPixel(1, mouse_x, mouse_y);

			// if (pixel_data != -1)
			// m_hoveredEntity = {static_cast<entt::entity>(pixel_data), m_scene.get()};
		}
		command_buffer.endRendering();
	}

	void EditorLayer::onEvent(Event &p_event)
	{
		if (m_viewportHovered)
			m_editorCamera.onEvent(p_event);

		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(EditorLayer::onKeyPressEvent));
		eventDispatcher.dispatch<MouseButtonPressEvent>(TST_BIND_EVENT_FN(EditorLayer::onMouseButtonPressEvent));
	}

	void EditorLayer::onUIRender()
	{
		#pragma region Setup Dockspace
		static bool               p_open          = true;
		static bool               opt_fullscreen  = true;
		static bool               opt_padding     = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport *viewport = ig::GetMainViewport();
			ig::SetNextWindowPos(viewport->WorkPos);
			ig::SetNextWindowSize(viewport->WorkSize);
			ig::SetNextWindowViewport(viewport->ID);
			ig::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ig::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ig::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ig::Begin("DockSpace Demo", &p_open, window_flags);

		if (!opt_padding)
			ig::PopStyleVar(); // ImGuiStyleVar_WindowPadding

		if (opt_fullscreen)
			ig::PopStyleVar(2); // ImGuiStyleVar_WindowRounding ImGuiStyleVar_WindowBorderSize

		// Submit the DockSpace
		ImGuiIO &   io        = ig::GetIO();
		ImGuiStyle &style     = ig::GetStyle();
		style.WindowMinSize.x = 300.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ig::GetID("MyDockSpace");
			ig::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = 30.0f;
		#pragma endregion

		if (ig::BeginMenuBar())
		{
			if (ig::BeginMenu("File"))
			{
				if (ig::MenuItem("New", "Ctrl+N"))
					newScene();
				if (ig::MenuItem("Save", "Ctrl+S"))
					saveScene();
				if (ig::MenuItem("Open", "Ctrl+O"))
					openScene();
				if (ig::MenuItem("Quit", "Ctrl+Q"))
					getApp().close();

				ig::Separator();
				ig::EndMenu();
			}
			ig::EndMenuBar();
		}

		ig::Begin("Settings");

		// ig::Text("Renderer2D quad count: %d", m_renderer2d->getStats().quadCount);
		ig::Text("Hovered entity tag: %s", m_hoveredEntity ? m_hoveredEntity.getComponent<TagComponent>().tag.c_str() : "Null");

		ig::End(); // Settings

		m_sceneHierarchyPanel->onUIRender();

		{
			ui::ScopedStyle window_padding{ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f)};

			ig::Begin("Viewport");

			m_viewportFocused = ig::IsWindowFocused();
			m_viewportHovered = ig::IsWindowHovered();
			((EditorApplication &) getApp()).setBlockUIEvents(!m_viewportFocused && !m_viewportHovered);

			ImVec2 viewport_offset = ig::GetCursorPos();
			auto   size            = ig::GetContentRegionAvail();
			m_viewportSize         = {size.x, size.y};

			ig::Image((VkDescriptorSet) m_renderTargetDescriptorSet, size, ImVec2(0, 1), ImVec2(1, 0));

			ImVec2 window_size = ig::GetWindowSize();
			ImVec2 min_bound   = ig::GetWindowPos();
			min_bound.x        += viewport_offset.x;
			// min_bound.y += viewport_offset.y;

			ImVec2 max_bound    = {min_bound.x + window_size.x, min_bound.y + window_size.y};
			m_viewportBounds[0] = {min_bound.x, min_bound.y};
			m_viewportBounds[1] = {max_bound.x, max_bound.y};

			Entity selected_entity = m_sceneHierarchyPanel->getSelectedEntity();

			if (selected_entity && m_gizmoType != -1)
			{
				auto w = ig::GetWindowWidth();
				auto h = ig::GetWindowHeight();

				igz::SetOrthographic(false);
				igz::SetDrawlist();
				igz::SetRect(ig::GetWindowPos().x, ig::GetWindowPos().y, w, h);

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

			ig::End(); // Viewport
		}

		ig::End(); // DockSpace Demo
	}

	void EditorLayer::newScene()
	{
		m_scene = make_reference<Scene>();
		m_scene->setViewportSize(static_cast<uint32>(m_viewportSize.x), static_cast<uint32>(m_viewportSize.y));
		m_sceneHierarchyPanel->setScene(m_scene);
	}

	void EditorLayer::saveScene()
	{
		auto save_location = os::saveFileDialog({{"Toaster Scene", "tscene"}});
		if (!save_location.empty())
		{
			SceneSerializer ss{m_scene};
			ss.serialize(save_location);
		}
	}

	void EditorLayer::openScene()
	{
		auto scene_location = os::openFileDialog({{"Toaster Scene", "tscene"}});
		if (!scene_location.empty())
		{
			newScene();

			SceneSerializer ss{m_scene};
			ss.deserialize(scene_location);

			LOG_INFO("{}", scene_location.string());
			auto &app = getApp();
			app.getWindow().setTitle(m_initialWindowTitle + " -> " + m_scene->getName());
		}
	}

	bool EditorLayer::onKeyPressEvent(KeyPressEvent &p_event)
	{
		if (m_viewportFocused)
		{
			if (m_viewportHovered && !input::isMouseButtonDown(input::EMouseButton::eRight))
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
		}

		if (p_event.getKeyCode() == input::EKeyCode::eEscape)
			getApp().close();

		return false;
	}

	bool EditorLayer::onMouseButtonPressEvent(MouseButtonPressEvent &p_event)
	{
		if (p_event.getMouseButton() == input::EMouseButton::eLeft)
			if (m_viewportHovered && !igz::IsOver() && !input::isKeyDown(input::EKeyCode::eLeftAlt))
				m_sceneHierarchyPanel->setSelectedEntity(m_hoveredEntity);

		return false;
	}

	void EditorLayer::_createRenderTargetResources(uint32 p_width, uint32 p_height)
	{
		m_renderTargetImage        = nullptr;
		m_renderTargetImageMemory  = nullptr;
		m_renderTargetImageView    = nullptr;
		m_renderTargetImageSampler = nullptr;

		m_renderTargetImageDescriptorInfo = vk::DescriptorImageInfo{};

		m_renderTargetDepthImage       = nullptr;
		m_renderTargetDepthImageMemory = nullptr;
		m_renderTargetDepthImageView   = nullptr;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		vk::Format colour_attachment_format{swapchain->getSurfaceFormat().format};

		ctx->createImage(p_width, p_height, colour_attachment_format, vk::ImageTiling::eOptimal,
						 vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_renderTargetImage,
						 m_renderTargetImageMemory);

		ctx->transitionImageLayout(m_renderTargetImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone,
								   vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput);

		m_renderTargetImageView = ctx->createImageView(m_renderTargetImage, colour_attachment_format, vk::ImageAspectFlagBits::eColor);

		auto physical_device_props = ctx->getPhysicalDevice().getProperties();

		vk::SamplerCreateInfo sampler_create_info{};
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.magFilter               = vk::Filter::eLinear;
		sampler_create_info.minFilter               = vk::Filter::eLinear;
		sampler_create_info.mipmapMode              = vk::SamplerMipmapMode::eLinear;
		sampler_create_info.addressModeU            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeV            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.addressModeW            = vk::SamplerAddressMode::eRepeat;
		sampler_create_info.mipLodBias              = 0.0f;
		sampler_create_info.anisotropyEnable        = true;
		sampler_create_info.maxAnisotropy           = physical_device_props.limits.maxSamplerAnisotropy;
		sampler_create_info.compareEnable           = false;
		sampler_create_info.compareOp               = vk::CompareOp::eAlways;
		sampler_create_info.minLod                  = 0.0f;
		sampler_create_info.maxLod                  = 0.0f;
		sampler_create_info.borderColor             = vk::BorderColor::eFloatCustomEXT;
		sampler_create_info.unnormalizedCoordinates = false;

		// This is purely aesthetic
		vk::SamplerCustomBorderColorCreateInfoEXT border_colour_create_info{};
		border_colour_create_info.customBorderColor = vk::ClearColorValue{1.0f, 0.0f, 1.0f, 1.0f};
		border_colour_create_info.format            = vk::Format::eR8G8B8A8Srgb;

		sampler_create_info.pNext = &border_colour_create_info;

		m_renderTargetImageSampler = {ctx->getDevice(), sampler_create_info};

		m_renderTargetImageDescriptorInfo.imageView   = m_renderTargetImageView;
		m_renderTargetImageDescriptorInfo.sampler     = m_renderTargetImageSampler;
		m_renderTargetImageDescriptorInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::Format depth_attachment_format{swapchain->getDepthFormat()};

		ctx->createImage(p_width, p_height, depth_attachment_format, vk::ImageTiling::eOptimal,
						 vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,
						 m_renderTargetDepthImage, m_renderTargetDepthImageMemory);

		ctx->transitionImageLayout(m_renderTargetDepthImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits::eNone,
								   vk::AccessFlagBits::eDepthStencilAttachmentWrite,
								   vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
								   vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests);

		m_renderTargetDepthImageView = ctx->createImageView(m_renderTargetDepthImage, depth_attachment_format, vk::ImageAspectFlagBits::eDepth);
	}
}
