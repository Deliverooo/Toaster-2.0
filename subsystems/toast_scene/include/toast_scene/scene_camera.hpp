#pragma once

#include "toast_scene.hpp"

#include "toast_lib/camera.hpp"

namespace toaster
{
	class TST_SCENE_API SceneCamera final : public Camera
	{
	public:
		enum class EProjectionType
		{
			ePerspective, eOrthographic
		};

		SceneCamera();

		auto setPerspective(float32 p_fov, float32 p_z_near = 0.1f, float32 p_z_far = 100.0f) -> void;
		auto setOrthographic(float32 p_size, float32 p_z_near = -1.0f, float32 p_z_far = 1.0f) -> void;
		auto setViewportSize(tsm::uint2 p_size) -> void;


		auto setPerspectiveFov(const float32 p_fov) -> void
		{
			m_perspectiveFov = p_fov;
			_recalculateProjection();
		}

		[[nodiscard]] auto getPerspectiveFov() const -> float32 { return m_perspectiveFov; }

		auto setPerspectiveNearClip(const float32 p_z_near) -> void
		{
			m_perspectiveNear = p_z_near;
			_recalculateProjection();
		}

		[[nodiscard]] auto getPerspectiveNearClip() const -> float32 { return m_perspectiveNear; }

		auto setPerspectiveFarClip(const float32 p_z_far) -> void
		{
			m_perspectiveFar = p_z_far;
			_recalculateProjection();
		}

		[[nodiscard]] auto getPerspectiveFarClip() const -> float32 { return m_perspectiveFar; }

		auto setOrthoSize(const float32 p_size) -> void
		{
			m_orthoSize = p_size;
			_recalculateProjection();
		}

		[[nodiscard]] auto getOrthoSize() const -> float32 { return m_orthoSize; }

		auto setOrthoNearClip(const float32 p_z_near) -> void
		{
			m_orthoNear = p_z_near;
			_recalculateProjection();
		}

		[[nodiscard]] auto getOrthoNearClip() const -> float32 { return m_orthoNear; }

		auto setOrthoFarClip(const float32 p_z_far) -> void
		{
			m_orthoFar = p_z_far;
			_recalculateProjection();
		}

		[[nodiscard]] auto getOrthoFarClip() const -> float32 { return m_orthoFar; }

		auto setProjectionType(EProjectionType p_type) -> void
		{
			m_projectionType = p_type;
			_recalculateProjection();
		}

		[[nodiscard]] auto getProjectionType() const -> EProjectionType { return m_projectionType; }

		[[nodiscard]] auto getAspectRatio() const -> float32 { return m_aspectRatio; }

	private:
		auto _recalculateProjection() -> void;

		EProjectionType m_projectionType{EProjectionType::eOrthographic};

		float32 m_perspectiveFov{tsm::radians(45.0f)};
		float32 m_perspectiveNear{0.1f};
		float32 m_perspectiveFar{1000.0f};

		float32 m_orthoSize{10.0f};
		float32 m_orthoNear{-1.0f};
		float32 m_orthoFar{1.0f};

		float32 m_aspectRatio{1920.0f / 1080.0f};
	};
}
