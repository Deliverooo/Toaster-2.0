#include "client_layer.hpp"

#include "toaster/toast_kernel/application.hpp"
#include "toaster/toast_kernel/input.hpp"
#include "toaster/toast_lib/io/file_stream.hpp"
#include "toaster/toast_render/globals.hpp"
#include "toaster/toast_render/renderer.hpp"

namespace toaster
{
	ClientLayer::ClientLayer(Application *p_app) : IAppLayer(p_app)
	{
	}

	void ClientLayer::onInit()
	{
		auto vertex_stage = io::filesystem::readFile("resources/shaders/fullscreen_quad.vert.glsl");
		auto pixel_stage  = io::filesystem::readFile("resources/shaders/fullscreen_quad.pixel.glsl");
		m_shader = gpu::IShader::create("Fullscreen", {{gpu::EShaderType::eVertex, vertex_stage.c_str()}, {gpu::EShaderType::ePixel, pixel_stage.c_str()}});

		m_material  = Material::create(m_shader);
	}

	void ClientLayer::onDestroy()
	{
	}

	void ClientLayer::onUpdate(const float32 p_dt)
	{
		m_time += p_dt;

		auto &app    = getApp();
		int32 width  = app.getWindow().getWidth();
		int32 height = app.getWindow().getHeight();
		RenderCommand::setViewport(0, 0, width, height);

		RenderCommand::clear();
		RenderCommand::clearColour(0.0f, 0.0f, 0.0f, 1.0f);

		m_material->setUniform("u_View", glm::mat4{1.0f});
		m_material->setUniform("u_Proj", glm::mat4{1.0f});

		m_material->setUniform("u_Res", glm::vec2{width, height});
		m_material->setUniform("u_Time", m_time);

		Renderer::submitFullscreenQuad(m_material);
	}

	void ClientLayer::onEvent(Event &p_event)
	{
		EventDispatcher eventDispatcher(p_event);
		eventDispatcher.dispatch<KeyPressEvent>(TST_BIND_EVENT_FN(ClientLayer::onKeyPressEvent));
	}

	bool ClientLayer::onKeyPressEvent(KeyPressEvent &e)
	{
		if (e.getKeyCode() == input::EKeyCode::eEscape)
		{
			getApp().close();
		}

		return false;
	}
}
