#pragma once

#include "math_vector.hpp"

namespace tsm
{
	inline auto decomposeTransform(const float4x4 &p_transform, float3 &p_out_translation, quat &p_out_orientation, float3 &p_out_scale) -> void
	{
		p_out_translation = float3(p_transform[3]);

		p_out_scale.x = glm::length(float3(p_transform[0]));
		p_out_scale.y = glm::length(float3(p_transform[1]));
		p_out_scale.z = glm::length(float3(p_transform[2]));

		const float3x3 rot_mat = {float3(p_transform[0]) / p_out_scale.x, p_transform[1] / p_out_scale.y, p_transform[2] / p_out_scale.z};

		p_out_orientation = glm::quat_cast(rot_mat);
	}
}
