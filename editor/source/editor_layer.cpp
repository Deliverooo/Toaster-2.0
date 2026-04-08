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
		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		m_scene               = make_reference<Scene>(ctx);
		m_sceneHierarchyPanel = make_reference<SceneHierarchyPanel>(ctx, m_scene);

		uint32 window_width{swapchain->getExtent().width};
		uint32 window_height{swapchain->getExtent().height};

		m_windowWidth  = window_width;
		m_windowHeight = window_height;

		m_editorCamera.setViewportSize(static_cast<float32>(m_windowWidth), static_cast<float32>(m_windowHeight));

		swapchain->addResizeCallback([this](uint32 width, uint32 height)
		{
			LOG_INFO("{}, {}", width, height);

			m_windowWidth  = width;
			m_windowHeight = height;

			_createAttachmentImages();

			m_editorCamera.setViewportSize(static_cast<float32>(m_windowWidth), static_cast<float32>(m_windowHeight));
			m_scene->setViewportSize(m_windowWidth, m_windowHeight);
			m_renderer2D->onResize(width, height);

			_createDescriptorSets();
		});

		m_compositeVertexBufferLayout = {{gpu::EShaderDataType::eFloat3, "a_Position"}, {gpu::EShaderDataType::eFloat2, "a_TexCoord"}};

		gpu::VKShader::Bytecode    vs_bytecode = io::filesystem::readBinary("shaders/composite.vert.glsl.spv");
		gpu::VKShader::Bytecode    ps_bytecode = io::filesystem::readBinary("shaders/composite.pixel.glsl.spv");
		gpu::VKShader::BytecodeMap shader_bytecode_map{{vk::ShaderStageFlagBits::eVertex, vs_bytecode}, {vk::ShaderStageFlagBits::eFragment, ps_bytecode}};
		m_compositeShader = make_reference<gpu::VKShader>(ctx, shader_bytecode_map);

		gpu::PipelineCreateInfo pipeline_create_info{};
		pipeline_create_info.vertexBufferLayout = m_compositeVertexBufferLayout;
		pipeline_create_info.colourAttachments  = {swapchain->getSurfaceFormat().format};
		pipeline_create_info.depthFormat        = {swapchain->getDepthFormat()};
		pipeline_create_info.shader             = m_compositeShader;
		m_compositePipeline                     = make_reference<gpu::VKPipeline>(ctx, pipeline_create_info);

		m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}});
		m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}});
		m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}});
		m_fullscreenQuadVertices.emplace_back(FullscreenQuadVertex{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}});
		m_fullscreenQuadIndices = {0, 1, 3, 1, 2, 3};

		vk::DeviceSize vbo_size{m_fullscreenQuadVertices.size() * sizeof(FullscreenQuadVertex)};
		m_fullscreenQuadVertexBuffer = make_reference<gpu::VKVertexBuffer>(ctx, m_fullscreenQuadVertices.data(), vbo_size);

		vk::DeviceSize ibo_size{m_fullscreenQuadIndices.size() * sizeof(uint16)};
		m_fullscreenQuadIndexBuffer = make_reference<gpu::VKIndexBuffer>(ctx, m_fullscreenQuadIndices.data(), ibo_size);

		Renderer2DCreateInfo renderer_2d_create_info{};
		renderer_2d_create_info.renderTargetWidth  = m_windowWidth;
		renderer_2d_create_info.renderTargetHeight = m_windowHeight;
		m_renderer2D                               = make_reference<Renderer2D>(ctx, renderer_2d_create_info);

		_createAttachmentImages();
		_createDescriptorPool();
		_createDescriptorSets();

		m_initialWindowTitle = app.getWindow().getTitle();
		app.getWindow().setTitle(m_initialWindowTitle + " -> " + m_scene->getName());

		Entity e{m_scene->createEntity()};
		auto & src{e.addComponent<SpriteRendererComponent>()};
		src.colour = {1.0f, 1.0f, 0.0f, 1.0f};
	}

	void EditorLayer::onDestroy()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		ctx->getDevice().waitIdle();
	}

	void EditorLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		auto &app       = getApp();
		auto  ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain = app.getWindow().getSwapchain();

		uint32 frame_index{swapchain->getFrameIndex()};
		uint32 image_index{swapchain->getImageIndex()};

		auto &command_buffer = swapchain->getCurrentCommandBuffer();

		// if (m_viewportFocused)
		m_editorCamera.onUpdate(p_dt);

		// m_scene->onUpdate(p_dt);

		// m_renderer2D->begin(command_buffer, frame_index, m_editorCamera.getViewMatrix(), m_editorCamera.getProjectionMatrix());
		// glm::mat4 quad_transform{glm::translate(glm::mat4{1.0f}, {0.0f, 0.0f, -1.0f}) * glm::scale(glm::mat4{1.0f}, glm::vec3{200.0f, 200.0f, 1.0f})};
		// m_renderer2D->submitQuad(quad_transform, {1.0f, 1.0f, 1.0f, 1.0f});
		// m_renderer2D->end(command_buffer, frame_index);

		m_scene->onRender(command_buffer, frame_index, p_dt, m_renderer2D, m_editorCamera.getViewMatrix(), m_editorCamera.getProjectionMatrix());

		{
			vk::RenderingAttachmentInfo colour_attachment_info{};
			colour_attachment_info.clearValue         = vk::ClearColorValue{0.005f, 0.005f, 0.005f, 1.0f};;
			colour_attachment_info.imageView          = m_colourAttachmentImageView;
			colour_attachment_info.imageLayout        = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			colour_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			colour_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eAverage;
			colour_attachment_info.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			colour_attachment_info.resolveImageView   = swapchain->getImageView(image_index);

			vk::RenderingAttachmentInfo depth_attachment_info{};
			depth_attachment_info.clearValue         = vk::ClearDepthStencilValue{1.0f, 0u};;
			depth_attachment_info.imageView          = m_depthAttachmentImageView;
			depth_attachment_info.imageLayout        = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.loadOp             = vk::AttachmentLoadOp::eClear;
			depth_attachment_info.storeOp            = vk::AttachmentStoreOp::eStore;
			depth_attachment_info.resolveMode        = vk::ResolveModeFlagBits::eMin;
			depth_attachment_info.resolveImageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depth_attachment_info.resolveImageView   = swapchain->getDepthImageView();

			vk::RenderingInfo rendering_info{};
			rendering_info.renderArea           = vk::Rect2D{{0, 0}, vk::Extent2D{m_windowWidth, m_windowHeight}};
			rendering_info.layerCount           = 1;
			rendering_info.colorAttachmentCount = 1;
			rendering_info.pColorAttachments    = &colour_attachment_info;
			rendering_info.pDepthAttachment     = &depth_attachment_info;

			vk::Viewport viewport{0.0f, 0.0f, static_cast<float32>(m_windowWidth), static_cast<float32>(m_windowHeight), 0.0f, 1.0f};
			vk::Rect2D   scissor{vk::Offset2D{0, 0}, vk::Extent2D{m_windowWidth, m_windowHeight}};

			command_buffer.beginRendering(rendering_info);
			command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_compositePipeline->getPipeline());
			command_buffer.setViewport(0, viewport);
			command_buffer.setScissor(0, scissor);

			command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_compositePipeline->getPipelineLayout(), 0, *m_compositeDescriptorSets[frame_index],
											  nullptr);
			m_fullscreenQuadVertexBuffer->bind(command_buffer);
			m_fullscreenQuadIndexBuffer->bind(command_buffer, vk::IndexType::eUint16);

			command_buffer.drawIndexed(m_fullscreenQuadIndices.size(), 1, 0, 0, 0);
			command_buffer.endRendering();
		}
	}

	void EditorLayer::onEvent(Event &p_event)
	{
		// if (m_viewportHovered)
		m_editorCamera.onEvent(p_event);

		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(EditorLayer::onKeyPressEvent));
		eventDispatcher.dispatch<MouseButtonPressEvent>(TST_BIND_EVENT_FN(EditorLayer::onMouseButtonPressEvent));
	}

	void EditorLayer::onUIRender()
	{
		#if 0
		auto & app       = getApp();
		auto   ctx       = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto   swapchain = app.getWindow().getSwapchain();
		uint32 frame_index{swapchain->getFrameIndex()};

		#pragma region Setup Dockspace
		static bool               p_open          = true;
		static bool               opt_fullscreen  = true;
		static bool               opt_padding     = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags          window_flags    = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
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

			ig::Image(static_cast<VkDescriptorSet>(*m_compositeDescriptorSets[frame_index]), size, ImVec2(0, 1), ImVec2(1, 0));

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

		#endif
	}

	void EditorLayer::newScene()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		m_scene   = make_reference<Scene>(ctx);
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

	void EditorLayer::_createAttachmentImages()
	{
		m_colourAttachmentImage       = nullptr;
		m_colourAttachmentImageMemory = nullptr;
		m_colourAttachmentImageView   = nullptr;

		m_depthAttachmentImage       = nullptr;
		m_depthAttachmentImageMemory = nullptr;
		m_depthAttachmentImageView   = nullptr;

		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());
		auto  swapchain{app.getWindow().getSwapchain()};

		vk::SampleCountFlagBits sample_count{ctx->getMaxUsableSampleCount()};

		{
			vk::Format colour_attachment_format{swapchain->getSurfaceFormat().format};
			ctx->createImage(m_windowWidth, m_windowHeight, 1, sample_count, colour_attachment_format, vk::ImageTiling::eOptimal,
							 vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
							 m_colourAttachmentImage, m_colourAttachmentImageMemory);

			ctx->transitionImageLayout(m_colourAttachmentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eNone,
									   vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eNone, vk::PipelineStageFlagBits::eColorAttachmentOutput, 1,
									   vk::ImageAspectFlagBits::eColor);

			m_colourAttachmentImageView = ctx->createImageView(m_colourAttachmentImage, colour_attachment_format, vk::ImageAspectFlagBits::eColor, 1);
		}
		{
			vk::Format depth_attachment_format{swapchain->getDepthFormat()};
			ctx->createImage(m_windowWidth, m_windowHeight, 1, sample_count, depth_attachment_format, vk::ImageTiling::eOptimal,
							 vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal,
							 m_depthAttachmentImage, m_depthAttachmentImageMemory);

			ctx->transitionImageLayout(m_depthAttachmentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits::eNone,
									   vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits::eNone,
									   vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, 1,
									   vk::ImageAspectFlagBits::eDepth);

			m_depthAttachmentImageView = ctx->createImageView(m_depthAttachmentImage, depth_attachment_format, vk::ImageAspectFlagBits::eDepth, 1);
		}
	}

	void EditorLayer::_createDescriptorPool()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		std::array<vk::DescriptorPoolSize, 2> descriptor_pool_sizes{};
		descriptor_pool_sizes[0].descriptorCount = 2 * gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[0].type            = vk::DescriptorType::eUniformBuffer;

		descriptor_pool_sizes[1].descriptorCount = 2 * gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_sizes[1].type            = vk::DescriptorType::eCombinedImageSampler;

		vk::DescriptorPoolCreateInfo descriptor_pool_create_info{};
		descriptor_pool_create_info.poolSizeCount = descriptor_pool_sizes.size();
		descriptor_pool_create_info.pPoolSizes    = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets       = gpu::VKGPUContext::c_maxFramesInFlight;
		descriptor_pool_create_info.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

		m_descriptorPool = {ctx->getDevice(), descriptor_pool_create_info};
	}

	void EditorLayer::_createDescriptorSets()
	{
		auto &app = getApp();
		auto  ctx = dynamic_cast<gpu::VKGPUContext *>(app.getWindow().getGPUContext());

		m_compositeDescriptorSets.clear();
		{
			auto &                        set_layout = m_compositeShader->getDescriptorSetLayout(0);
			std::vector                   descriptor_set_layouts{gpu::VKGPUContext::c_maxFramesInFlight, *set_layout};
			vk::DescriptorSetAllocateInfo descriptor_set_allocate_info{};
			descriptor_set_allocate_info.descriptorPool     = m_descriptorPool;
			descriptor_set_allocate_info.descriptorSetCount = gpu::VKGPUContext::c_maxFramesInFlight;
			descriptor_set_allocate_info.pSetLayouts        = descriptor_set_layouts.data();

			m_compositeDescriptorSets = ctx->getDevice().allocateDescriptorSets(descriptor_set_allocate_info);

			for (uint32 i{0u}; i < gpu::VKGPUContext::c_maxFramesInFlight; ++i)
			{
				std::array<vk::WriteDescriptorSet, 1> write_descriptor_sets{};
				write_descriptor_sets[0].descriptorCount = 1;
				write_descriptor_sets[0].descriptorType  = vk::DescriptorType::eCombinedImageSampler;
				write_descriptor_sets[0].pImageInfo      = &m_renderer2D->getRenderTargetDescriptorImageInfo();
				write_descriptor_sets[0].dstSet          = m_compositeDescriptorSets[i];
				write_descriptor_sets[0].dstBinding      = 0;
				write_descriptor_sets[0].dstArrayElement = 0;

				ctx->getDevice().updateDescriptorSets(write_descriptor_sets, {});
			}
		}
	}
}
