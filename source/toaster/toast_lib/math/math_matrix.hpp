#pragma once

#include "math_vector.hpp"

namespace tsm
{
	inline void decomposeTransform(const float4x4 &p_transform, float3 &p_translation, quat &p_orientation, float3 &p_scale)
	{
		p_translation = float3(p_transform[3]);

		p_scale.x = glm::length(float3(p_transform[0]));
		p_scale.y = glm::length(float3(p_transform[1]));
		p_scale.z = glm::length(float3(p_transform[2]));

		float3x3 rot_mat = {float3(p_transform[0]) / p_scale.x, p_transform[1] / p_scale.y, p_transform[2] / p_scale.z};

		p_orientation = glm::quat_cast(rot_mat);
	}
}
