/*!
 * @file camera.hpp
 */
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace toaster
{
	/*!
	 * @class Camera
	 * @brief A camera class that only holds an arbitrary projection matrix
	 *
	 * @note This class is not designed to be used on its own, as it doesn't provide any projection calculations.
	 *		 See @file ortho_camera.hpp for a concrete implementation.
	 */
	class Camera
	{
	public:
		virtual ~Camera() = default;

		Camera() = default;

		explicit Camera(const glm::mat4 &p_projection) : m_projectionMatrix(p_projection)
		{
		};

		[[nodiscard]] const glm::mat4 &getProjectionMatrix() const { return m_projectionMatrix; }

	protected:
		glm::mat4 m_projectionMatrix{1.0f};
	};
}
