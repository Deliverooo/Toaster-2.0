#include "math/vector4.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	constexpr Vector4 Vector4::operator+(Vector4 val) const
	{
		return {x + val.x, y + val.y, z + val.z, w + val.w};
	}

	constexpr void Vector4::operator+=(const Vector4 &val)
	{
		x += val.x;
		y += val.y;
		z += val.z;
		w += val.w;
	}

	constexpr Vector4 Vector4::operator-(const Vector4 &val) const
	{
		return {x - val.x, y - val.y, z - val.z, w - val.w};
	}

	constexpr void Vector4::operator-=(const Vector4 &val)
	{
		x -= val.x;
		y -= val.y;
		z -= val.z;
		w -= val.w;
	}

	constexpr Vector4 Vector4::operator*(const Vector4 &val) const
	{
		return {x * val.x, y * val.y, z * val.z, w * val.w};
	}

	constexpr Vector4 Vector4::operator*(Float val) const
	{
		return {x * val, y * val, z * val, w * val};
	}

	constexpr void Vector4::operator*=(Float val)
	{
		x *= val;
		y *= val;
		z *= val;
		w *= val;
	}

	constexpr Vector4 Vector4::operator/(const Vector4 &val) const
	{
		return {x / val.x, y / val.y, z / val.z, w / val.w};
	}

	constexpr Vector4 Vector4::operator/(Float val) const
	{
		return {x / val, y / val, z / val, w / val};
	}

	constexpr void Vector4::operator/=(Float val)
	{
		x /= val;
		y /= val;
		z /= val;
		w /= val;
	}

	constexpr Vector4 Vector4::operator-() const
	{
		return {-x, -y, -z, -w};
	}

	constexpr bool Vector4::operator==(const Vector4 &val) const
	{
		return x == val.x && y == val.y && z == val.z && w == val.w;
	}

	constexpr bool Vector4::operator!=(const Vector4 &val) const
	{
		return x != val.x || y != val.y || z != val.z || w != val.w;
	}

	Vector4::operator std::string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]"};
	}

	std::string Vector4::to_string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]"};
	}

	void Vector4::normalize()
	{
		x /= length();
		y /= length();
		z /= length();
		w /= length();
	}

	bool Vector4::isNormalized() const
	{
		return length() == 1.0f;
	}

	Vector4 Vector4::normalized() const
	{
		return {x / length(), y / length(), z / length(), w / length()};
	}

	Float Vector4::length() const
	{
		return math::sqrt(x * x + y * y + z * z + w * w);
	}

	Float Vector4::dot(const Vector4 &v) const
	{
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	Float Vector4::angle(const Vector4 &v) const
	{
		return math::acos(dot(v));
	}
}
