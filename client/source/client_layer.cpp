#include "client_layer.hpp"

#include "application.hpp"
#include "globals.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "events/key_event.hpp"
#include "events/mouse_event.hpp"
#include "io/file_stream.hpp"

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app), m_camera(-1.0f, 1.0f, -p_app->getWindow().getAspect(), p_app->getWindow().getAspect(), 0.1f, 100.0f)
	{
		io::filesystem::setWorkingDirectory("../../../"); // The main Toaster dir (where the resource folder is)

		m_texture = gpu::Texture2D::create("resources/textures/Orbo_02.png");

		auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->bind();
		quad_shader->setUniform("u_Texture", 0);

		m_mesh = Mesh::importFromFile("resources/meshes/Orbo.fbx");

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

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		RenderCommand::clearColour({0.2f, 0.3f, 0.3f, 1.0f});
		RenderCommand::clear();

		if (input::isKeyDown(input::EKeyCode::eA))
			m_camera.setPosition(m_camera.getPosition() + glm::vec3{-1.0f * p_dt, 0.0f, 0.0f});
		if (input::isKeyDown(input::EKeyCode::eD))
			m_camera.setPosition(m_camera.getPosition() + glm::vec3{1.0f * p_dt, 0.0f, 0.0f});
		if (input::isKeyDown(input::EKeyCode::eW))
			m_camera.setPosition(m_camera.getPosition() + glm::vec3{0.0f, 1.0f * p_dt, 0.0f});
		if (input::isKeyDown(input::EKeyCode::eS))
			m_camera.setPosition(m_camera.getPosition() + glm::vec3{0.0f, -1.0f * p_dt, 0.0f});

		if (input::isKeyDown(input::EKeyCode::eUp))
			m_camera.setPosition(m_camera.getPosition() + glm::vec3{0.0f, 0.0f, 1.0f * p_dt});
		if (input::isKeyDown(input::EKeyCode::eDown))
			m_camera.setPosition(m_camera.getPosition() + glm::vec3{0.0f, 0.0f, -1.0f * p_dt});

		if (input::isKeyDown(input::EKeyCode::eR))
			m_camera.setRotation(m_camera.getRotation() + 1.0f * p_dt);

		auto quad_shader = Globals::shaderLibrary()->get("Quad");
		quad_shader->bind();

		quad_shader->setUniform("u_View", m_camera.getViewMatrix());
		quad_shader->setUniform("u_Proj", m_camera.getProjectionMatrix());

		quad_shader->setUniform("u_Time", m_time);
		quad_shader->setUniform("u_Resolution", {getApp().getWindow().getWidth(), getApp().getWindow().getHeight()});

		m_texture->bind();

		for (uint32 i = 0u; i < 10; i++)
		{
			for (uint32 j = 0u; j < 10; j++)
			{
				Renderer::submitGeometry(Globals::quadVertexArray(), quad_shader,
										 glm::translate(glm::mat4(1.0f), {j * 0.3f, i * 0.3f, -1.0f}) * glm::scale(glm::mat4(1.0f), glm::vec3{0.25f}));
			}
		}

		Renderer::submitMesh(m_mesh, m_camera.getViewMatrix(), m_camera.getProjectionMatrix(),
							 glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3{10.0f}), glm::radians(-90.0f), glm::vec3{1.0f, 0.0f, 0.0f}));
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<MouseMoveEvent>([&](MouseMoveEvent &e)
		{
			// Stuff...
			return false;
		});

		eventDispatcher.dispatch<WindowResizeEvent>([&](WindowResizeEvent &e)
		{
			RenderCommand::setViewport({0, 0, e.getWidth(), e.getHeight()});

			return false;
		});

		eventDispatcher.dispatch<KeyPressEvent>([&](KeyPressEvent &e)
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
		});
	}
}
