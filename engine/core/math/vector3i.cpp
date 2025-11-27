#include "math/vector3i.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	constexpr Vector3I Vector3I::operator+(Vector3I val) const
	{
		return {x + val.x, y + val.y, z + val.z};
	}

	constexpr void Vector3I::operator+=(const Vector3I &val)
	{
		x += val.x;
		y += val.y;
		z += val.z;
	}

	constexpr Vector3I Vector3I::operator-(const Vector3I &val) const
	{
		return {x - val.x, y - val.y, z - val.z};
	}

	constexpr void Vector3I::operator-=(const Vector3I &val)
	{
		x -= val.x;
		y -= val.y;
		z -= val.z;
	}

	constexpr Vector3I Vector3I::operator*(const Vector3I &val) const
	{
		return {x * val.x, y * val.y, z * val.z};
	}

	constexpr Vector3I Vector3I::operator*(int32 val) const
	{
		return {x * val, y * val, z * val};
	}

	constexpr void Vector3I::operator*=(int32 val)
	{
		x *= val;
		y *= val;
		z *= val;
	}

	constexpr Vector3I Vector3I::operator/(const Vector3I &val) const
	{
		return {x / val.x, y / val.y, z / val.z};
	}

	constexpr Vector3I Vector3I::operator/(int32 val) const
	{
		return {x / val, y / val, z / val};
	}

	constexpr void Vector3I::operator/=(int32 val)
	{
		x /= val;
		y /= val;
		z /= val;
	}

	constexpr Vector3I Vector3I::operator-() const
	{
		return {-x, -y, -z};
	}

	constexpr bool Vector3I::operator==(const Vector3I &val) const
	{
		return x == val.x && y == val.y && z == val.z;
	}

	constexpr bool Vector3I::operator!=(const Vector3I &val) const
	{
		return x != val.x || y != val.y && z != val.z;
	}

	Vector3I::operator String() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]"};
	}

	String Vector3I::to_string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]"};
	}

	float32 Vector3I::length() const
	{
		return math::sqrt(static_cast<float>(x * x + y * y + z * z));
	}

	float32 Vector3I::dot(const Vector3I &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	float32 Vector3I::angle(const Vector3I &v) const
	{
		return math::acos(dot(v));
	}
}
