#include "scene_camera.hpp"

#include "toaster/toast_lib/toast_assert.h"

namespace toaster
{
	SceneCamera::SceneCamera()
	{
		_recalculateProjection();
	}

	void SceneCamera::setPerspective(float32 p_fov, float32 p_z_near, float32 p_z_far)
	{
		m_projectionType  = EProjectionType::ePerspective;
		m_perspectiveFov  = p_fov;
		m_perspectiveNear = p_z_near;
		m_perspectiveFar  = p_z_far;
		_recalculateProjection();
	}

	void SceneCamera::setOrthographic(float32 p_size, float32 p_z_near, float32 p_z_far)
	{
		m_projectionType = EProjectionType::eOrthographic;
		m_orthoSize      = p_size;
		m_orthoNear      = p_z_near;
		m_orthoFar       = p_z_far;
		_recalculateProjection();
	}

	void SceneCamera::setViewportSize(uint32 p_width, uint32 p_height)
	{
		TST_ASSERT_MSG(p_width > 0 && p_height > 0, "No");
		m_aspectRatio = static_cast<float32>(p_width) / static_cast<float32>(p_height);
		_recalculateProjection();
	}

	void SceneCamera::_recalculateProjection()
	{
		switch (m_projectionType)
		{
			case EProjectionType::ePerspective:
			{
				m_projection = glm::perspective(m_perspectiveFov, m_aspectRatio, m_perspectiveNear, m_perspectiveFar);
				break;
			}
			case EProjectionType::eOrthographic:
			{
				const float32 ortho_left   = -m_orthoSize * m_aspectRatio * 0.5f;
				const float32 ortho_right  = m_orthoSize * m_aspectRatio * 0.5f;
				const float32 ortho_bottom = -m_orthoSize * 0.5f;
				const float32 ortho_top    = m_orthoSize * 0.5f;

				m_projection = glm::ortho(ortho_left, ortho_right, ortho_bottom, ortho_top, m_orthoNear, m_orthoFar);
				break;
			}
		}
	}
}
