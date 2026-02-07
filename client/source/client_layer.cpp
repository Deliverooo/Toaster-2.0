#include "client_layer.hpp"

#include <iostream>
#include <openglhpp/opengl.hpp>

#include "application.hpp"
#include "globals.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "events/key_event.hpp"
#include "events/mouse_event.hpp"
#include "io/file_stream.hpp"

namespace shaders::opengl
{
	#include "triangle.vert.glsl.spv.gl.inl"
	#include "triangle.pixel.glsl.spv.gl.inl"
}

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app_parent) : IAppLayer(p_app_parent), m_lastX(static_cast<float32>(p_app_parent->getWindow().getWidth()) / 2.0f),
														  m_lastY(static_cast<float32>(p_app_parent->getWindow().getHeight()) / 2.0f)
	{
		input::setCursorMode(input::ECursorMode::eDisabled);
	}

	ClientLayer::~ClientLayer()
	{
	}

	void ClientLayer::onInit()
	{
	}

	void ClientLayer::onDestroy()
	{
	}

	void ClientLayer::onUpdate(float32 p_dt)
	{
		m_time += p_dt;

		gl::clear(gl::ClearMaskBits::eColor | gl::ClearMaskBits::eDepth);
		gl::clearColor(0.2f, 0.3f, 0.3f, 1.0f);

		if (input::isKeyDown(input::EKeyCode::eW))
			m_camera.processKeyboard(Camera::EMovement::eForward, p_dt);
		if (input::isKeyDown(input::EKeyCode::eS))
			m_camera.processKeyboard(Camera::EMovement::eBackward, p_dt);
		if (input::isKeyDown(input::EKeyCode::eD))
			m_camera.processKeyboard(Camera::EMovement::eRight, p_dt);
		if (input::isKeyDown(input::EKeyCode::eA))
			m_camera.processKeyboard(Camera::EMovement::eLeft, p_dt);
		if (input::isKeyDown(input::EKeyCode::eLeftShift))
			m_camera.processKeyboard(Camera::EMovement::eDown, p_dt);
		if (input::isKeyDown(input::EKeyCode::eSpace))
			m_camera.processKeyboard(Camera::EMovement::eUp, p_dt);

		auto shader = gpu::Globals::quadShader();
		shader->bind();

		shader->setUniform("u_View", m_camera.getViewMatrix());
		shader->setUniform("u_Proj", m_camera.getProjectionMatrix(16.0f / 9.0f));

		shader->setUniform("u_Time", m_time);
		shader->setUniform("u_Resolution", {m_appParent->getWindow().getWidth(), m_appParent->getWindow().getHeight()});

		Renderer::submitQuad(glm::vec3(0.0f, 0.0f, 0.0f));

		Renderer::submitQuad(glm::vec3(0.0f, 1.0f, 1.0f));
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		static float32 lastX = 0.0f;
		static float32 lastY = 0.0f;

		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<MouseMoveEvent>([&](MouseMoveEvent &e)
		{
			if (m_firstMouse)
			{
				lastX        = e.getMouseX();
				lastY        = e.getMouseY();
				m_firstMouse = false;
			}

			double mouseDx = e.getMouseX() - lastX;
			double mouseDy = lastY - e.getMouseY();
			lastX          = e.getMouseX();
			lastY          = e.getMouseY();

			m_camera.processMouseMovement(mouseDx, mouseDy);

			return false;
		});

		eventDispatcher.dispatch<WindowResizedEvent>([&](WindowResizedEvent &e)
		{
			gl::viewport(0, 0, e.getWidth(), e.getHeight());

			return true;
		});

		eventDispatcher.dispatch<KeyPressedEvent>([&](KeyPressedEvent &e)
		{
			if (e.getKeyCode() == input::EKeyCode::eEscape)
			{
				m_appParent->close();
			}

			return false;
		});
	}
}
