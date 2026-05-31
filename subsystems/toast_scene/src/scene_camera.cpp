#include "toast_scene/scene_camera.hpp"

namespace toaster
{
	SceneCamera::SceneCamera()
	{
		_recalculateProjection();
	}

	auto SceneCamera::setPerspective(float32 p_fov, float32 p_z_near, float32 p_z_far) -> void
	{
		m_projectionType  = EProjectionType::ePerspective;
		m_perspectiveFov  = p_fov;
		m_perspectiveNear = p_z_near;
		m_perspectiveFar  = p_z_far;
		_recalculateProjection();
	}

	auto SceneCamera::setOrthographic(float32 p_size, float32 p_z_near, float32 p_z_far) -> void
	{
		m_projectionType = EProjectionType::eOrthographic;
		m_orthoSize      = p_size;
		m_orthoNear      = p_z_near;
		m_orthoFar       = p_z_far;
		_recalculateProjection();
	}

	auto SceneCamera::setViewportSize(tsm::uint2 p_size) -> void
	{
		// For the scene, we have to make sure that the width is not 0 when adding a camera component
		const float32 safe_width  = static_cast<float32>(std::max(p_size.x, 1u));
		const float32 safe_height = static_cast<float32>(std::max(p_size.y, 1u));

		m_aspectRatio = safe_width / safe_height;
		_recalculateProjection();
	}

	auto SceneCamera::_recalculateProjection() -> void
	{
		switch (m_projectionType)
		{
			case EProjectionType::ePerspective:
			{
				Dx::XMMATRIX proj{Dx::XMMatrixPerspectiveFovLH(m_perspectiveFov, m_aspectRatio, m_perspectiveNear, m_perspectiveFar)};
				proj = Dx::XMMatrixMultiply(proj, Dx::XMMatrixScaling(1.0f, -1.0f, 1.0f));
				Dx::XMStoreFloat4x4(&m_projection, proj);

				break;
			}
			case EProjectionType::eOrthographic:
			{
				const float32 ortho_left{-m_orthoSize * m_aspectRatio * 0.5f};
				const float32 ortho_right{m_orthoSize * m_aspectRatio * 0.5f};
				const float32 ortho_bottom{-m_orthoSize * 0.5f};
				const float32 ortho_top{m_orthoSize * 0.5f};

				Dx::XMMATRIX proj{Dx::XMMatrixOrthographicOffCenterLH(ortho_left, ortho_right, ortho_bottom, ortho_top, m_orthoNear, m_orthoFar)};
				Dx::XMStoreFloat4x4(&m_projection, proj);

				break;
			}
		}
	}
}
