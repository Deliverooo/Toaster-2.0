/*!
 * @file ortho_camera.cpp
 */
#include "ortho_camera.hpp"

namespace toaster
{
	OrthoCamera::OrthoCamera(float32 p_left, float32 p_right, float32 p_bottom, float32 p_top, float32 p_near,
							 float32 p_far) : m_projectionMatrix(glm::ortho(p_left, p_right, p_bottom, p_top, p_near, p_far))
	{
	}

	const glm::mat4 &OrthoCamera::getViewMatrix() const
	{
		return m_viewMatrix;
	}

	const glm::vec3 &OrthoCamera::getPosition() const
	{
		return m_position;
	}

	float32 OrthoCamera::getRotation() const
	{
		return m_rotation;
	}

	void OrthoCamera::setPosition(const glm::vec3 &p_position)
	{
		m_position = p_position;
		recalculateViewMatrix();
	}

	void OrthoCamera::setRotation(float32 p_rotation)
	{
		m_rotation = p_rotation;
		recalculateViewMatrix();
	}

	void OrthoCamera::setProjectionMatrix(float32 p_left, float32 p_right, float32 p_bottom, float32 p_top, float32 p_near, float32 p_far)
	{
		m_projectionMatrix = glm::ortho(p_left, p_right, p_bottom, p_top, p_near, p_far);
	}

	void OrthoCamera::recalculateViewMatrix()
	{
		m_viewMatrix = glm::inverse(glm::translate(glm::mat4{1.0f}, m_position) * glm::rotate(glm::mat4{1.0f}, glm::radians(m_rotation), glm::vec3{0.0f, 0.0f, 1.0f}));
	}
}
