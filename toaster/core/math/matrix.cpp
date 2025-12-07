#include "matrix.hpp"

#include "math_funcs.hpp"

namespace tsm
{
	mat4x4f orthoProjD3DStyle(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		float xScale = 1.0f / (right - left);
		float yScale = 1.0f / (top - bottom);
		float zScale = 1.0f / (zFar - zNear);
		return mat4x4f(2.0f * xScale, 0, 0, 0, 0, 2.0f * yScale, 0, 0, 0, 0, zScale, 0, -(left + right) * xScale, -(bottom + top) * yScale, -zNear * zScale, 1);
	}

	mat4x4f orthoProjOGLStyle(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		float xScale = 1.0f / (right - left);
		float yScale = 1.0f / (top - bottom);
		float zScale = 1.0f / (zFar - zNear);
		return mat4x4f(2.0f * xScale, 0, 0, 0, 0, 2.0f * yScale, 0, 0, 0, 0, -2.0f * zScale, 0, -(left + right) * xScale, -(bottom + top) * yScale,
					   -(zNear + zFar) * zScale, 1);
	}

	mat4x4f perspProjD3DStyle(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		float xScale = 1.0f / (right - left);
		float yScale = 1.0f / (top - bottom);
		float zScale = 1.0f / (zFar - zNear);
		return mat4x4f(2.0f * xScale, 0, 0, 0, 0, 2.0f * yScale, 0, 0, -(left + right) * xScale, -(bottom + top) * yScale, zFar * zScale, 1, 0, 0, -zNear * zFar * zScale,
					   0);
	}

	mat4x4f perspProjOGLStyle(float left, float right, float bottom, float top, float zNear, float zFar)
	{
		float xScale = 1.0f / (right - left);
		float yScale = 1.0f / (top - bottom);
		float zScale = 1.0f / (zFar - zNear);
		return mat4x4f(2.0f * zNear * xScale, 0, 0, 0, 0, 2.0f * zNear * yScale, 0, 0, (left + right) * xScale, (bottom + top) * yScale, -(zNear + zFar) * zScale, -1, 0,
					   0, -2.0f * zNear * zFar * zScale, 0);
	}

	mat4x4f perspProjD3DStyleReverse(float left, float right, float bottom, float top, float zNear)
	{
		float xScale = 1.0f / (right - left);
		float yScale = 1.0f / (top - bottom);

		return mat4x4f(2.0f * xScale, 0, 0, 0, 0, 2.0f * yScale, 0, 0, -(left + right) * xScale, -(bottom + top) * yScale, 0, 1, 0, 0, zNear, 0);
	}

	mat4x4f perspProjD3DStyle(float verticalFOV, float aspect, float zNear, float zFar)
	{
		float yScale = 1.0f / tanf(0.5f * verticalFOV);
		float xScale = yScale / aspect;
		float zScale = 1.0f / (zFar - zNear);
		return mat4x4f(xScale, 0, 0, 0, 0, yScale, 0, 0, 0, 0, zFar * zScale, 1, 0, 0, -zNear * zFar * zScale, 0);
	}

	mat4x4f perspProjOGLStyle(float verticalFOV, float aspect, float zNear, float zFar)
	{
		float yScale = 1.0f / tanf(0.5f * verticalFOV);
		float xScale = yScale / aspect;
		float zScale = 1.0f / (zFar - zNear);
		return mat4x4f(xScale, 0, 0, 0, 0, yScale, 0, 0, 0, 0, -(zNear + zFar) * zScale, -1, 0, 0, -2.0f * zNear * zFar * zScale, 0);
	}

	mat4x4f perspProjD3DStyleReverse(float verticalFOV, float aspect, float zNear)
	{
		float yScale = 1.0f / tanf(0.5f * verticalFOV);
		float xScale = yScale / aspect;

		return mat4x4f(xScale, 0, 0, 0, 0, yScale, 0, 0, 0, 0, 0, 1, 0, 0, zNear, 0);
	}

	mat4x4f translate(const mat4x4f &m, const vec4f &translation)
	{
		mat4x4f result = m;
		result[3][0] += translation.x;
		result[3][1] += translation.y;
		result[3][2] += translation.z;
		return result;
	}

	mat4x4f translate(const mat4x4f &m, const vec3f &translation)
	{
		mat4x4f result = m;
		result[3][0] += translation.x;
		result[3][1] += translation.y;
		result[3][2] += translation.z;
		return result;
	}

	mat4x4f rotate(const mat4x4f &m, float angleRadians, const vec3f &axis)
	{
		mat4x4f result = m;
		float32 c      = cos(angleRadians);
		float32 s      = sin(angleRadians);

		return result;
	}

	mat4x4f scale(const mat4x4f &m, const vec3f &scaleFactors)
	{
		mat4x4f result = m;
		result[0][0] *= scaleFactors.x;
		result[1][1] *= scaleFactors.y;
		result[2][2] *= scaleFactors.z;
		return result;
	}
}
