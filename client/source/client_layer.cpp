#include "client_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app), m_cameraController(16.0f / 9.0f, true)
	{
	}

	void ClientLayer::onInit()
	{
		io::filesystem::setWorkingDirectory("../../../"); // The main Toaster dir (where the resource folder is)
		m_texture  = gpu::Texture2D::create("resources/textures/Orbo_02.png");
		m_texture2 = gpu::Texture2D::create("resources/textures/0001.png");

		Renderer2DCreateInfo renderer_2d_create_info;
		renderer_2d_create_info.maxQuads = 10000u;
		m_renderer2d                     = make_reference<Renderer2D>(renderer_2d_create_info);

		input::setCursorMode(input::ECursorMode::eDisabled);
	}

	void ClientLayer::onDestroy()
	{
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		RenderCommand::clearColour({0.2f, 0.3f, 0.3f, 1.0f});
		RenderCommand::clear();

		m_cameraController.onUpdate(p_dt);

		m_renderer2d->begin(m_cameraController.getCamera().getViewMatrix(), m_cameraController.getCamera().getProjectionMatrix());


		// m_renderer2d->submitQuad({2.0f, 0.0f, -0.1f}, {1.0f, 1.0f}, m_texture);
		m_renderer2d->submitQuad({0.0f, 1.0f, -0.1f}, {1.0f, 1.0f}, m_texture2);
		m_renderer2d->submitQuad({1.0f, 0.5f, -0.1f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f});

		m_renderer2d->end();
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<MouseMoveEvent>(TST_BIND_EVENT_FN(ClientLayer::onMouseMoveEvent));
		eventDispatcher.dispatch<WindowResizeEvent>(TST_BIND_EVENT_FN(ClientLayer::onWindowResizeEvent));
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));

		m_cameraController.onEvent(p_event);
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eI)
		{
			if (input::getCursorMode() == input::ECursorMode::eDisabled)
			{
				input::setCursorMode(input::ECursorMode::eNormal);
			}
			else
			{
				input::setCursorMode(input::ECursorMode::eDisabled);
			}
		}

		if (e.getKeyCode() == input::EKeyCode::eEscape)
		{
			getApp().close();
		}

		return false;
	}

	bool ClientLayer::onMouseMoveEvent(MouseMoveEvent &e)
	{
		// Stuff...
		return false;
	}

	bool ClientLayer::onWindowResizeEvent(WindowResizeEvent &e)
	{
		RenderCommand::setViewport({0, 0, e.getWidth(), e.getHeight()});

		return false;
	}
}
