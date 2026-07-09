#include "toast_kernel/fp_camera.hpp"
#include "toast_kernel/input.hpp"

namespace toaster
{
	FPCamera::FPCamera(InputContext *p_ctx, float32 p_fov, float32 p_aspectRatio, float32 p_near, float32 p_far) : m_ctx(p_ctx), m_fov(p_fov),
																												   m_aspectRatio(p_aspectRatio), m_zNear(p_near),
																												   m_zFar(p_far)
	{
		_updateProjection();
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

			Dx::XMVECTOR delta_position{Dx::XMVectorZero()};
			if (m_ctx->isKeyDown(input::EKeyCode::eW))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			if (m_ctx->isKeyDown(input::EKeyCode::eA))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
			if (m_ctx->isKeyDown(input::EKeyCode::eS))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			if (m_ctx->isKeyDown(input::EKeyCode::eD))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

			Dx::XMVECTOR position{Dx::XMLoadFloat3(&m_position)};

			position = Dx::XMVectorAdd(position, Dx::XMVectorScale(Dx::XMVector3Transform(Dx::XMVector3NormalizeSafe(delta_position), getRotationMatrix()),
																   speed * p_dt));

			if (m_ctx->isKeyDown(input::EKeyCode::eSpace))
				position = Dx::XMVectorSubtract(position, Dx::XMVectorScale(c_upDir, speed * p_dt));
			if (m_ctx->isKeyDown(input::EKeyCode::eLeftShift))
				position = Dx::XMVectorAdd(position, Dx::XMVectorScale(c_upDir, speed * p_dt));

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

	auto FPCamera::onResize(tsm::uint2 p_size) -> void
	{
		m_aspectRatio = p_size.aspect();
		_updateProjection();
	}

	auto FPCamera::getViewMatrix() const -> Dx::XMMATRIX
	{
		Dx::XMVECTOR position{getPosition()};
		return Dx::XMMatrixLookToLH(position, getForwardDirection(), c_upDir);
	}

	auto FPCamera::getRotationMatrix() const -> Dx::XMMATRIX
	{
		return Dx::XMMatrixRotationRollPitchYaw(m_pitch, -m_yaw, 0.0f);
	}

	auto FPCamera::getViewProjection() const -> Dx::XMMATRIX
	{
		return Dx::XMMatrixMultiply(Dx::XMLoadFloat4x4(&m_projection), getViewMatrix());
	}

	auto FPCamera::getForwardDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_forwardDir, getRotationMatrix());
	}

	auto FPCamera::getRightDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_rightDir, getRotationMatrix());
	}

	auto FPCamera::getUpDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_upDir, getRotationMatrix());
	}

	auto FPCamera::getPosition() const -> Dx::XMVECTOR
	{
		return Dx::XMLoadFloat3(&m_position);
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
		setPerspective(tsm::radians(m_fov) * m_zoom, m_aspectRatio, m_zNear, m_zFar);
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

	auto FirstPersonCameraEntity::onCreate(void *p_user_data) -> void
	{
		auto &params{*static_cast<FirstPersonCameraEntityCreateParams *>(p_user_data)};
		m_ctx         = params.p_ctx;
		m_fov         = params.p_fov;
		m_aspectRatio = params.p_aspectRatio;
		m_zNear       = params.p_near;
		m_zFar        = params.p_far;

		m_camera          = &getOrAddComponent<scene::CameraComponent>();
		m_camera->primary = true;
		m_transform       = &getComponent<scene::TransformComponent>();

		_updateProjection();
	}

	auto FirstPersonCameraEntity::onUpdate(float32 p_dt) -> void
	{
		if (!m_ctx)
			return;
		if (m_ctx->isMouseButtonDown(input::EMouseButton::eRight))
		{
			if (m_ctx->getCursorMode() != input::ECursorMode::eDisabled)
				m_ctx->setCursorMode(input::ECursorMode::eDisabled);

			float32 speed{m_ctx->isKeyDown(input::EKeyCode::eLeftControl) ? 30.0f : 10.0f};

			Dx::XMVECTOR delta_position{Dx::XMVectorZero()};
			if (m_ctx->isKeyDown(input::EKeyCode::eW))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f));
			if (m_ctx->isKeyDown(input::EKeyCode::eA))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f));
			if (m_ctx->isKeyDown(input::EKeyCode::eS))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
			if (m_ctx->isKeyDown(input::EKeyCode::eD))
				delta_position = Dx::XMVectorAdd(delta_position, Dx::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));

			Dx::XMVECTOR position{Dx::XMLoadFloat3(&m_transform->translation)};

			position = Dx::XMVectorAdd(position, Dx::XMVectorScale(Dx::XMVector3Transform(Dx::XMVector3NormalizeSafe(delta_position), getRotationMatrix()),
																   speed * p_dt));

			if (m_ctx->isKeyDown(input::EKeyCode::eSpace))
				position = Dx::XMVectorAdd(position, Dx::XMVectorScale(c_upDir, speed * p_dt));
			if (m_ctx->isKeyDown(input::EKeyCode::eLeftShift))
				position = Dx::XMVectorSubtract(position, Dx::XMVectorScale(c_upDir, speed * p_dt));

			Dx::XMStoreFloat3(&m_transform->translation, position);

			const tsm::float2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			const tsm::float2 delta{(mouse - m_initialMousePosition) * 0.002f};
			m_yaw   += delta.x;
			m_pitch -= delta.y;
			if (m_pitch > tsm::radians(89.0f))
				m_pitch = tsm::radians(89.0f);
			if (m_pitch < tsm::radians(-89.0f))
				m_pitch = tsm::radians(-89.0f);

			m_initialMousePosition = mouse;

			Dx::XMStoreFloat4(&m_transform->orientation, Dx::XMQuaternionRotationMatrix(getRotationMatrix()));
		}
		else
		{
			if (m_ctx->getCursorMode() != input::ECursorMode::eNormal)
				m_ctx->setCursorMode(input::ECursorMode::eNormal);

			const tsm::float2 mouse{m_ctx->getMouseX(), m_ctx->getMouseY()};
			m_initialMousePosition = mouse;
		}
	}

	auto FirstPersonCameraEntity::onEvent(Event &p_event) -> void
	{
		EventDispatcher dispatcher{p_event};
		dispatcher.dispatch<MouseScrollEvent>(TST_BIND_EVENT_FN(FirstPersonCameraEntity::_onMouseScrollEvent));
	}

	auto FirstPersonCameraEntity::onResize(tsm::uint2 p_size) -> void
	{
		m_camera->camera.setViewportSize(p_size);
	}

	auto FirstPersonCameraEntity::getViewMatrix() const -> Dx::XMMATRIX
	{
		Dx::XMVECTOR position{getPosition()};
		return Dx::XMMatrixLookAtLH(position, Dx::XMVectorAdd(position, getForwardDirection()), Dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	}

	auto FirstPersonCameraEntity::getProjectionMatrix() const -> Dx::XMMATRIX
	{
		return m_camera->camera.getProjectionMatrix();
	}

	auto FirstPersonCameraEntity::getRotationMatrix() const -> Dx::XMMATRIX
	{
		return Dx::XMMatrixRotationRollPitchYaw(m_pitch, -m_yaw, 0.0f);
	}

	auto FirstPersonCameraEntity::getViewProjection() const -> Dx::XMMATRIX
	{
		return Dx::XMMatrixMultiply(m_camera->camera.getProjectionMatrix(), getViewMatrix());
	}

	auto FirstPersonCameraEntity::getForwardDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_forwardDir, getRotationMatrix());
	}

	auto FirstPersonCameraEntity::getRightDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_rightDir, getRotationMatrix());
	}

	auto FirstPersonCameraEntity::getUpDirection() const -> Dx::XMVECTOR
	{
		return Dx::XMVector3Transform(c_upDir, getRotationMatrix());
	}

	auto FirstPersonCameraEntity::getPosition() const -> Dx::XMVECTOR
	{
		return Dx::XMLoadFloat3(&m_transform->translation);
	}

	auto FirstPersonCameraEntity::getPitch() const -> float32
	{
		return m_pitch;
	}

	auto FirstPersonCameraEntity::getYaw() const -> float32
	{
		return m_yaw;
	}

	auto FirstPersonCameraEntity::_updateProjection() -> void
	{
		m_camera->camera.setPerspective(tsm::radians(m_fov) * m_zoom, m_zNear, m_zFar);
	}

	auto FirstPersonCameraEntity::_onMouseScrollEvent(MouseScrollEvent &p_event) -> bool
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
