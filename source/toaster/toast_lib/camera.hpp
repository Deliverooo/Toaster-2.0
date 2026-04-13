#pragma once

#include "system_types.h"
#include "math/math_vector.hpp"

namespace toaster
{
	class Camera
	{
	public:
		Camera() = default;

		Camera(const tsm::float4x4 &p_projection) : m_projection{p_projection}
		{
		}

		virtual ~Camera() = default;

		auto               setProjectionMatrix(const tsm::float4x4 &p_projection) -> void { m_projection = p_projection; }
		[[nodiscard]] auto getProjectionMatrix() const -> const tsm::float4x4 & { return m_projection; }

		auto setPerspective(float32 p_fov, float32 p_aspect, float32 p_z_near, float32 p_z_far) -> void
		{
			m_projection = glm::perspective(p_fov, p_aspect, p_z_near, p_z_far);
		}

	protected:
		tsm::float4x4 m_projection{1.0f};
	};
}
