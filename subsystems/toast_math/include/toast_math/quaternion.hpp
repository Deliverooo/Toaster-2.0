#pragma once

#include "math_matrix.hpp"

namespace tsm
{
	template<typename Type>
	struct Quat
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

		constexpr Quat() : x(static_cast<Type>(0)), y(static_cast<Type>(0)), z(static_cast<Type>(0)), w(static_cast<Type>(1))
		{
		}

		constexpr Quat(Type p_w, Type p_x, Type p_y, Type p_z) : x(p_x), y(p_y), z(p_z), w(p_w)
		{
		}

		constexpr Quat(Type p_w, const Vec3<Type> &p_v) : x(p_v.x), y(p_v.y), z(p_v.z), w(p_w)
		{
		}

		constexpr auto operator*(const Quat &p_other) const -> Quat<Type>
		{
			return Quat{
				w * p_other.w - x * p_other.x - y * p_other.y - z * p_other.z,
				w * p_other.x + x * p_other.w + y * p_other.z - z * p_other.y,
				w * p_other.y - x * p_other.z + y * p_other.w + z * p_other.x,
				w * p_other.z + x * p_other.y - y * p_other.x + z * p_other.w
			};
		}
	};

	template<typename Type>
	constexpr auto toMat3(const Quat<Type> &p_quat) -> Mat3<Type>
	{
		Mat3<Type> result(Type(1));
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
	constexpr auto toMat4(const Quat<Type> &p_quat) -> Mat4<Type>
	{
		return Mat4<Type>{toMat3(p_quat)};
	}

	template<typename Type>
	constexpr auto quatCast(const Mat3<Type> &p_m) -> Quat<Type>
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
			default: assert(false);
				return {1, 0, 0, 0};
		}
	}

	template<typename Type>
	constexpr auto quatCast(const Mat4<Type> &p_m) -> Quat<Type>
	{
		return quatCast(Mat3{p_m});
	}

	template<typename Type>
	constexpr auto axisAngle(Type p_angle, Vec3<Type> p_axis) -> Quat<Type>
	{
		Type length = std::sqrtf(p_axis.x * p_axis.x + p_axis.y * p_axis.y + p_axis.z * p_axis.z);
		if (length > 0.0f)
		{
			p_axis.x /= length;
			p_axis.y /= length;
			p_axis.z /= length;
		}

		Type half_angle = p_angle * 0.5f;
		Type sin_half   = std::sinf(half_angle);

		return Quat<Type>{std::cosf(half_angle), p_axis.x * sin_half, p_axis.y * sin_half, p_axis.z * sin_half};
	}

	template<typename Type>
	constexpr auto fromYawPitchRoll(Type p_yaw, Type p_pitch, Type p_roll) -> Quat<Type>
	{
		return axisAngle(p_yaw, Vec3<Type>::unitX) * axisAngle(p_pitch, Vec3<Type>::unitY) * axisAngle(p_roll, Vec3<Type>::unitZ);
	}

	using quatf = Quat<f32>;
	using quatd = Quat<f64>;

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
