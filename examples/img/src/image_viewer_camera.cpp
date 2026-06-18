#include "img/image_viewer_camera.hpp"

namespace img
{
	ImageViewerCamera::ImageViewerCamera(tst::InputContext *p_ctx, float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far) : m_ctx(p_ctx), m_fov(p_fov),
																																		  m_aspectRatio(p_aspectRatio),
																																		  m_zNear(p_near), m_zFar(p_far)
	{
		_updateProjection();
	}

	auto ImageViewerCamera::onUpdate(float32 p_dt) -> void
	{
		if (!m_ctx)
			return;
		if (m_ctx->isMouseButtonDown(tst::input::EMouseButton::eRight))
		{
			if (m_ctx->getCursorMode() != tst::input::ECursorMode::eDisabled)
				m_ctx->setCursorMode(tst::input::ECursorMode::eDisabled);

			float32 speed{m_ctx->isKeyDown(tst::input::EKeyCode::eLeftControl) ? 30.0f : 10.0f};

			Dx::XMVECTOR delta_position{Dx::XMVectorZero()};
			if (m_ctx->isKeyDown(tst::input::EKeyCode::eW))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			if (m_ctx->isKeyDown(tst::input::EKeyCode::eA))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
			if (m_ctx->isKeyDown(tst::input::EKeyCode::eS))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			if (m_ctx->isKeyDown(tst::input::EKeyCode::eD))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));

			Dx::XMVECTOR position{Dx::XMLoadFloat3(&m_position)};

			position = Dx::XMVectorAdd(position, Dx::XMVectorScale(Dx::XMVector3Transform(Dx::XMVector3NormalizeSafe(delta_position), getRotationMatrix()),
																   speed * p_dt));

			if (m_ctx->isKeyDown(tst::input::EKeyCode::eSpace))
				position = Dx::XMVectorAdd(position, Dx::XMVectorScale(c_upDir, speed * p_dt));
			if (m_ctx->isKeyDown(tst::input::EKeyCode::eLeftShift))
				position = Dx::XMVectorSubtract(position, Dx::XMVectorScale(c_upDir, speed * p_dt));

			Dx::XMStoreFloat3(&m_position, position);

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
			if (m_ctx->getCursorMode() != tst::input::ECursorMode::eNormal)
				m_ctx->setCursorMode(tst::input::ECursorMode::eNormal);

			const tsm::float2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			m_initialMousePosition = mouse;
		}
	}

	auto ImageViewerCamera::onEvent(tst::Event &p_event) -> void
	{
		tst::EventDispatcher dispatcher{p_event};
		dispatcher.dispatch<tst::MouseScrollEvent>(TST_BIND_EVENT_FN(ImageViewerCamera::_onMouseScrollEvent));
	}

	auto ImageViewerCamera::onResize(tsm::uint2 p_size) -> void
	{
		m_aspectRatio = p_size.aspect();
		_updateProjection();
	}

	auto ImageViewerCamera::getViewMatrix() const -> Dx::XMMATRIX
	{
		Dx::XMVECTOR position{getPosition()};
		return Dx::XMMatrixLookToLH(position, getForwardDirection(), c_upDir);
	}

	auto ImageViewerCamera::getRotationMatrix() const -> Dx::XMMATRIX
	{
		return Dx::XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);
	}

	auto ImageViewerCamera::getViewProjection() const -> Dx::XMMATRIX
	{
		return Dx::XMMatrixMultiply(Dx::XMLoadFloat4x4(&m_projection), getViewMatrix());
	}

	auto ImageViewerCamera::getForwardDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_forwardDir, getRotationMatrix());
	}

	auto ImageViewerCamera::getRightDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_rightDir, getRotationMatrix());
	}

	auto ImageViewerCamera::getUpDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_upDir, getRotationMatrix());
	}

	auto ImageViewerCamera::getPosition() const -> Dx::XMVECTOR
	{
		return Dx::XMLoadFloat3(&m_position);
	}

	auto ImageViewerCamera::getPitch() const -> float32
	{
		return m_pitch;
	}

	auto ImageViewerCamera::getYaw() const -> float32
	{
		return m_yaw;
	}

	auto ImageViewerCamera::_updateProjection() -> void
	{
		setPerspective(tsm::radians(m_fov) * m_zoom, m_aspectRatio, m_zNear, m_zFar);
	}

	auto ImageViewerCamera::_onMouseScrollEvent(tst::MouseScrollEvent &p_event) -> bool
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
