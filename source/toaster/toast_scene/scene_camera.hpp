#pragma once

#include "toaster/toast_lib/camera.hpp"

namespace toaster
{
	class SceneCamera : public Camera
	{
	public:
		enum class EProjectionType
		{
			ePerspective, eOrthographic
		};

		SceneCamera();

		void setPerspective(float32 p_fov, float32 p_z_near = 0.1f, float32 p_z_far = 100.0f);
		void setOrthographic(float32 p_size, float32 p_z_near = -1.0f, float32 p_z_far = 1.0f);
		void setViewportSize(uint32 p_width, uint32 p_height);

		void setPerspectiveFov(const float32 p_fov)
		{
			m_perspectiveFov = p_fov;
			_recalculateProjection();
		}

		[[nodiscard]] float32 getPerspectiveFov() const { return m_perspectiveFov; }

		void setPerspectiveNearClip(const float32 p_z_near)
		{
			m_perspectiveNear = p_z_near;
			_recalculateProjection();
		}

		[[nodiscard]] float32 getPerspectiveNearClip() const { return m_perspectiveNear; }

		void setPerspectiveFarClip(const float32 p_z_far)
		{
			m_perspectiveFar = p_z_far;
			_recalculateProjection();
		}

		[[nodiscard]] float32 getPerspectiveFarClip() const { return m_perspectiveFar; }

		void setOrthoSize(const float32 p_size)
		{
			m_orthoSize = p_size;
			_recalculateProjection();
		}

		[[nodiscard]] float32 getOrthoSize() const { return m_orthoSize; }

		void setOrthoNearClip(const float32 p_z_near)
		{
			m_orthoNear = p_z_near;
			_recalculateProjection();
		}

		[[nodiscard]] float32 getOrthoNearClip() const { return m_orthoNear; }

		void setOrthoFarClip(const float32 p_z_far)
		{
			m_orthoFar = p_z_far;
			_recalculateProjection();
		}

		[[nodiscard]] float32 getOrthoFarClip() const { return m_orthoFar; }

		void setProjectionType(EProjectionType p_type)
		{
			m_projectionType = p_type;
			_recalculateProjection();
		}

		[[nodiscard]] EProjectionType getProjectionType() const { return m_projectionType; }

		[[nodiscard]] float32 getAspectRatio() const { return m_aspectRatio; }

	private:
		void _recalculateProjection();

		EProjectionType m_projectionType{EProjectionType::eOrthographic};

		float32 m_perspectiveFov{glm::radians(45.0f)};

		float32 m_perspectiveNear{0.1f};

		float32 m_perspectiveFar{1000.0f};

		float32 m_orthoSize{10.0f};
		float32 m_orthoNear{-1.0f};
		float32 m_orthoFar{1.0f};

		float32 m_aspectRatio{0.0f};
	};
}
