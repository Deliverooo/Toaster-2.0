#include "math/vector3.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	constexpr Vector3 Vector3::operator+(Vector3 val) const
	{
		return {x + val.x, y + val.y, z + val.z};
	}

	constexpr void Vector3::operator+=(const Vector3 &val)
	{
		x += val.x;
		y += val.y;
		z += val.z;
	}

	constexpr Vector3 Vector3::operator-(const Vector3 &val) const
	{
		return {x - val.x, y - val.y, z - val.z};
	}

	constexpr void Vector3::operator-=(const Vector3 &val)
	{
		x -= val.x;
		y -= val.y;
		z -= val.z;
	}

	constexpr Vector3 Vector3::operator*(const Vector3 &val) const
	{
		return {x * val.x, y * val.y, z * val.z};
	}

	constexpr Vector3 Vector3::operator*(Float val) const
	{
		return {x * val, y * val, z * val};
	}

	constexpr void Vector3::operator*=(Float val)
	{
		x *= val;
		y *= val;
		z *= val;
	}

	constexpr Vector3 Vector3::operator/(const Vector3 &val) const
	{
		return {x / val.x, y / val.y, z / val.z};
	}

	constexpr Vector3 Vector3::operator/(Float val) const
	{
		return {x / val, y / val, z / val};
	}

	constexpr void Vector3::operator/=(Float val)
	{
		x /= val;
		y /= val;
		z /= val;
	}

	constexpr Vector3 Vector3::operator-() const
	{
		return {-x, -y, -z};
	}

	constexpr bool Vector3::operator==(const Vector3 &val) const
	{
		return x == val.x && y == val.y && z == val.z;
	}

	constexpr bool Vector3::operator!=(const Vector3 &val) const
	{
		return x != val.x || y != val.y || z != val.z;
	}

	Vector3::operator std::string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]"};
	}

	std::string Vector3::to_string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]"};
	}

	void Vector3::normalize()
	{
		x /= length();
		y /= length();
		z /= length();
	}

	bool Vector3::isNormalized() const
	{
		return length() == 1.0f;
	}

	Vector3 Vector3::normalized() const
	{
		return {x / length(), y / length(), z / length()};
	}

	Float Vector3::length() const
	{
		return math::sqrt(x * x + y * y + z * z);
	}

	Float Vector3::dot(const Vector3 &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Float Vector3::angle(const Vector3 &v) const
	{
		return math::acos(dot(v));
	}
}
