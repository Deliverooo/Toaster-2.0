#pragma once

#include "math_matrix.hpp"

namespace tsm
{
	template<typename Type>
	struct quat
	{
		union
		{
			Type data[4];

			struct
			{
				Type x;
				Type y;
				Type z;
				Type w;
			};
		};

		constexpr quat() : x(static_cast<Type>(0)), y(static_cast<Type>(0)), z(static_cast<Type>(0)), w(static_cast<Type>(1))
		{
		}

		constexpr quat(Type p_w, Type p_x, Type p_y, Type p_z) : x(p_x), y(p_y), z(p_z), w(p_w)
		{
		}

		constexpr quat(Type p_w, const vec3<Type> &p_v) : x(p_v.x), y(p_v.y), z(p_v.z), w(p_w)
		{
		}
	};

	template<typename Type>
	constexpr auto toMat3(const quat<Type> &p_quat) -> mat3<Type>
	{
		mat3<Type> result(Type(1));
		Type       qxx{p_quat.x * p_quat.x};
		Type       qyy{p_quat.y * p_quat.y};
		Type       qzz{p_quat.z * p_quat.z};
		Type       qxz{p_quat.x * p_quat.z};
		Type       qxy{p_quat.x * p_quat.y};
		Type       qyz{p_quat.y * p_quat.z};
		Type       qwx{p_quat.w * p_quat.x};
		Type       qwy{p_quat.w * p_quat.y};
		Type       qwz{p_quat.w * p_quat.z};

		result[0][0] = Type(1) - Type(2) * (qyy + qzz);
		result[0][1] = Type(2) * (qxy + qwz);
		result[0][2] = Type(2) * (qxz - qwy);

		result[1][0] = Type(2) * (qxy - qwz);
		result[1][1] = Type(1) - Type(2) * (qxx + qzz);
		result[1][2] = Type(2) * (qyz + qwx);

		result[2][0] = Type(2) * (qxz + qwy);
		result[2][1] = Type(2) * (qyz - qwx);
		result[2][2] = Type(1) - Type(2) * (qxx + qyy);
		return result;
	}

	template<typename Type>
	constexpr auto toMat4(const quat<Type> &p_quat) -> mat4<Type>
	{
		return mat4<Type>{toMat3(p_quat)};
	}

	template<typename Type>
	constexpr auto quatCast(const mat3<Type> &p_m) -> quat<Type>
	{
		Type fourXSquaredMinus1 = p_m[0][0] - p_m[1][1] - p_m[2][2];
		Type fourYSquaredMinus1 = p_m[1][1] - p_m[0][0] - p_m[2][2];
		Type fourZSquaredMinus1 = p_m[2][2] - p_m[0][0] - p_m[1][1];
		Type fourWSquaredMinus1 = p_m[0][0] + p_m[1][1] + p_m[2][2];

		int  biggestIndex             = 0;
		Type fourBiggestSquaredMinus1 = fourWSquaredMinus1;
		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex             = 1;
		}
		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex             = 2;
		}
		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex             = 3;
		}

		Type biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<Type>(1)) * static_cast<Type>(0.5);
		Type mult       = static_cast<Type>(0.25) / biggestVal;

		switch (biggestIndex)
		{
			case 0:
				return {biggestVal, (p_m[1][2] - p_m[2][1]) * mult, (p_m[2][0] - p_m[0][2]) * mult, (p_m[0][1] - p_m[1][0]) * mult};
			case 1:
				return {(p_m[1][2] - p_m[2][1]) * mult, biggestVal, (p_m[0][1] + p_m[1][0]) * mult, (p_m[2][0] + p_m[0][2]) * mult};
			case 2:
				return {(p_m[2][0] - p_m[0][2]) * mult, (p_m[0][1] + p_m[1][0]) * mult, biggestVal, (p_m[1][2] + p_m[2][1]) * mult};
			case 3:
				return {(p_m[0][1] - p_m[1][0]) * mult, (p_m[2][0] + p_m[0][2]) * mult, (p_m[1][2] + p_m[2][1]) * mult, biggestVal};
			default: TST_ASSERT(false);
				return {1, 0, 0, 0};
		}
	}

	template<typename Type>
	constexpr auto quatCast(const mat4<Type> &p_m) -> quat<Type>
	{
		return quatCast(mat3{p_m});
	}

	template<typename Type>
	constexpr auto axisAngle(Type p_angle, const vec3<Type> &p_axis) -> quat<Type>
	{
		const Type a(p_angle);
		const Type s = std::sin(a * static_cast<Type>(0.5));

		return quat<Type>(std::cos(a * static_cast<Type>(0.5)), p_axis * s);
	}

	using quatf = quat<float32>;
	using quatd = quat<float64>;

	constexpr auto decomposeTransform(const float4x4 &p_transform, float3 &p_out_translation, quatf &p_out_orientation, float3 &p_out_scale) -> void
	{
		p_out_translation = p_transform[3];

		p_out_scale.x = length(float3(p_transform[0]));
		p_out_scale.y = length(float3(p_transform[1]));
		p_out_scale.z = length(float3(p_transform[2]));

		const float3x3 rot_mat = {float3(p_transform[0]) / p_out_scale.x, p_transform[1] / p_out_scale.y, p_transform[2] / p_out_scale.z};

		p_out_orientation = quatCast(rot_mat);
	}
}
