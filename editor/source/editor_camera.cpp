#include "editor_camera.hpp"

#include "toast_kernel/input.hpp"
#include "toast_lib/logging.hpp"

namespace toaster
{
	FPCamera::FPCamera(InputContext *p_ctx, float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far) : Camera(glm::perspective(glm::radians(p_fov),
																																				   p_aspectRatio, p_near,
																																				   p_far)), m_ctx(p_ctx),
																														   m_fov(p_fov), m_aspectRatio(p_aspectRatio),
																														   m_zNear(p_near), m_zFar(p_far)
	{
	}

	auto FPCamera::onUpdate(float32 p_dt) -> void
	{
		if (m_ctx->isMouseButtonDown(input::EMouseButton::eRight))
		{
			if (m_ctx->getCursorMode() != input::ECursorMode::eDisabled)
				m_ctx->setCursorMode(input::ECursorMode::eDisabled);

			float32 speed{m_ctx->isKeyDown(input::EKeyCode::eLeftControl) ? 30.0f : 10.0f};

			glm::vec3 delta_position{0.0f};
			if (m_ctx->isKeyDown(input::EKeyCode::eW))
				delta_position += c_forwardDir;
			if (m_ctx->isKeyDown(input::EKeyCode::eA))
				delta_position -= c_rightDir;
			if (m_ctx->isKeyDown(input::EKeyCode::eS))
				delta_position -= c_forwardDir;
			if (m_ctx->isKeyDown(input::EKeyCode::eD))
				delta_position += c_rightDir;

			delta_position = ((glm::length(delta_position) == 0.0f) ? glm::vec3{0.0f} : glm::normalize(delta_position)) * p_dt;
			m_position     += glm::vec3{getRotationMatrix() * glm::vec4{delta_position, 0.0f}} * speed;
			if (m_ctx->isKeyDown(input::EKeyCode::eSpace))
				m_position += c_upDir * p_dt * speed;
			if (m_ctx->isKeyDown(input::EKeyCode::eLeftShift))
				m_position -= c_upDir * p_dt * speed;

			const glm::vec2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			const glm::vec2 delta{(mouse - m_initialMousePosition) * 0.002f};
			m_yaw   += delta.x;
			m_pitch -= delta.y;
			if (m_pitch > glm::radians(89.0f))
				m_pitch = glm::radians(89.0f);
			if (m_pitch < glm::radians(-89.0f))
				m_pitch = glm::radians(-89.0f);

			m_initialMousePosition = mouse;
		}
		else
		{
			if (m_ctx->getCursorMode() != input::ECursorMode::eNormal)
				m_ctx->setCursorMode(input::ECursorMode::eNormal);

			const glm::vec2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			m_initialMousePosition = mouse;
		}
	}

	auto FPCamera::onEvent(Event &p_event) -> void
	{
		EventDispatcher dispatcher{p_event};
		dispatcher.dispatch<MouseScrollEvent>(TST_BIND_EVENT_FN(EditorCamera::_onMouseScrollEvent));
	}

	auto FPCamera::setViewportSize(float32 p_width, float32 p_height) -> void
	{
		m_aspectRatio = p_width / p_height;
		_updateProjection();
	}

	auto FPCamera::getViewMatrix() const -> glm::mat4
	{
		const glm::mat4 cameraTranslation{glm::translate(glm::mat4{1.0f}, m_position)};
		const glm::mat4 cameraRotation{getRotationMatrix()};
		return glm::inverse(cameraTranslation * cameraRotation);
	}

	auto FPCamera::getRotationMatrix() const -> glm::mat4
	{
		glm::quat pitchRotation{glm::angleAxis(m_pitch, glm::vec3{1.0f, 0.0f, 0.0f})};
		glm::quat yawRotation{glm::angleAxis(m_yaw, glm::vec3{0.0f, -1.0f, 0.0f})};

		return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
	}

	auto FPCamera::getViewProjection() const -> glm::mat4
	{
		return m_projection * getViewMatrix();
	}

	auto FPCamera::getForwardDirection() const -> glm::vec3
	{
		return getRotationMatrix() * glm::vec4{c_forwardDir, 0.0f};
	}

	auto FPCamera::getRightDirection() const -> glm::vec3
	{
		return getRotationMatrix() * glm::vec4{c_rightDir, 0.0f};
	}

	auto FPCamera::getUpDirection() const -> glm::vec3
	{
		return getRotationMatrix() * glm::vec4{c_upDir, 0.0f};
	}

	auto FPCamera::getPosition() const -> const glm::vec3 &
	{
		return m_position;
	}

	auto FPCamera::getPitch() const -> float32
	{
		return m_pitch;
	}

	auto FPCamera::getYaw() const -> float32
	{
		return m_yaw;
	}

	auto FPCamera::_updateProjection() -> void
	{
		m_projection = glm::perspective(glm::radians(m_fov) * m_zoom, m_aspectRatio, m_zNear, m_zFar);
	}

	auto FPCamera::_onMouseScrollEvent(MouseScrollEvent &p_event) -> bool
	{
		m_zoom -= p_event.getScrollY() * 0.02f;
		if (m_zoom < glm::radians(1.0f))
			m_zoom = glm::radians(1.0f);
		if (m_zoom > glm::radians(89.0f))
			m_zoom = glm::radians(89.0f);
		_updateProjection();

		return false;
	}
}
