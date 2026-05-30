#include "fp_camera.hpp"

#include "toast_kernel/input.hpp"
#include "toast_lib/logging.hpp"
#include "toast_lib/math/quaternion.hpp"
#include "toast_lib/math/trig.hpp"

namespace toaster
{
	FPCamera::FPCamera(InputContext *p_ctx, float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far) : Camera(tsm::perspective(tsm::radians(p_fov),
																																		   p_aspectRatio, p_near, p_far)),
																												   m_ctx(p_ctx), m_fov(p_fov),
																												   m_aspectRatio(p_aspectRatio), m_zNear(p_near),
																												   m_zFar(p_far)
	{
	}

	auto FPCamera::onUpdate(float32 p_dt) -> void
	{
		if (!m_ctx)
			return;
		if (m_ctx->isMouseButtonDown(input::EMouseButton::eRight))
		{
			if (m_ctx->getCursorMode() != input::ECursorMode::eDisabled)
				m_ctx->setCursorMode(input::ECursorMode::eDisabled);

			float32 speed{m_ctx->isKeyDown(input::EKeyCode::eLeftControl) ? 30.0f : 10.0f};

			tsm::float3 delta_position{0.0f};
			if (m_ctx->isKeyDown(input::EKeyCode::eW))
				delta_position += c_forwardDir;
			if (m_ctx->isKeyDown(input::EKeyCode::eA))
				delta_position -= c_rightDir;
			if (m_ctx->isKeyDown(input::EKeyCode::eS))
				delta_position -= c_forwardDir;
			if (m_ctx->isKeyDown(input::EKeyCode::eD))
				delta_position += c_rightDir;

			delta_position = ((tsm::length(delta_position) == 0.0f) ? tsm::float3{0.0f} : tsm::normalize(delta_position)) * p_dt;
			m_position     += tsm::float3{getRotationMatrix() * tsm::float4{delta_position, 0.0f}} * speed;
			if (m_ctx->isKeyDown(input::EKeyCode::eSpace))
				m_position += c_upDir * p_dt * speed;
			if (m_ctx->isKeyDown(input::EKeyCode::eLeftShift))
				m_position -= c_upDir * p_dt * speed;

			// LOG_INFO("Pos: {}", m_position);

			const tsm::float2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			const tsm::float2 delta{(mouse - m_initialMousePosition) * 0.002f};
			m_yaw   += delta.x;
			m_pitch -= delta.y;
			if (m_pitch > tsm::radians(89.0f))
				m_pitch = tsm::radians(89.0f);
			if (m_pitch < tsm::radians(-89.0f))
				m_pitch = tsm::radians(-89.0f);

			m_initialMousePosition = mouse;
		}
		else
		{
			if (m_ctx->getCursorMode() != input::ECursorMode::eNormal)
				m_ctx->setCursorMode(input::ECursorMode::eNormal);

			const tsm::float2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			m_initialMousePosition = mouse;
		}
	}

	auto FPCamera::onEvent(Event &p_event) -> void
	{
		EventDispatcher dispatcher{p_event};
		dispatcher.dispatch<MouseScrollEvent>(TST_BIND_EVENT_FN(FPCamera::_onMouseScrollEvent));
	}

	auto FPCamera::setViewportSize(float32 p_width, float32 p_height) -> void
	{
		m_aspectRatio = p_width / p_height;
		_updateProjection();
	}

	auto FPCamera::getViewMatrix() const -> tsm::float4x4
	{
		const tsm::float4x4 cameraTranslation{tsm::translate(tsm::float4x4{1.0f}, m_position)};
		const tsm::float4x4 cameraRotation{getRotationMatrix()};
		return tsm::inverse(cameraTranslation * cameraRotation);
	}

	auto FPCamera::getRotationMatrix() const -> tsm::float4x4
	{
		tsm::quatf pitchRotation{tsm::axisAngle(m_pitch, tsm::float3{1.0f, 0.0f, 0.0f})};
		tsm::quatf yawRotation{tsm::axisAngle(m_yaw, tsm::float3{0.0f, -1.0f, 0.0f})};

		return tsm::toMat4(yawRotation) * tsm::toMat4(pitchRotation);
	}

	auto FPCamera::getViewProjection() const -> tsm::float4x4
	{
		return m_projection * getViewMatrix();
	}

	auto FPCamera::getForwardDirection() const -> tsm::float3
	{
		return getRotationMatrix() * tsm::float4{c_forwardDir, 0.0f};
	}

	auto FPCamera::getRightDirection() const -> tsm::float3
	{
		return getRotationMatrix() * tsm::float4{c_rightDir, 0.0f};
	}

	auto FPCamera::getUpDirection() const -> tsm::float3
	{
		return getRotationMatrix() * tsm::float4{c_upDir, 0.0f};
	}

	auto FPCamera::getPosition() const -> const tsm::float3 &
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
		m_projection = tsm::perspective(tsm::radians(m_fov) * m_zoom, m_aspectRatio, m_zNear, m_zFar);
	}

	auto FPCamera::_onMouseScrollEvent(MouseScrollEvent &p_event) -> bool
	{
		m_zoom -= p_event.getScrollY() * 0.02f;
		if (m_zoom < tsm::radians(1.0f))
			m_zoom = tsm::radians(1.0f);
		if (m_zoom > tsm::radians(89.0f))
			m_zoom = tsm::radians(89.0f);
		_updateProjection();

		return false;
	}
}
