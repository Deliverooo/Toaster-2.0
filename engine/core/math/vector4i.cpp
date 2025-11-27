#include "math/vector4i.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	constexpr Vector4I Vector4I::operator+(Vector4I val) const
	{
		return {x + val.x, y + val.y, z + val.z, w + val.w};
	}

	constexpr void Vector4I::operator+=(const Vector4I &val)
	{
		x += val.x;
		y += val.y;
		z += val.z;
		w += val.w;
	}

	constexpr Vector4I Vector4I::operator-(const Vector4I &val) const
	{
		return {x - val.x, y - val.y, z - val.z, w - val.w};
	}

	constexpr void Vector4I::operator-=(const Vector4I &val)
	{
		x -= val.x;
		y -= val.y;
		z -= val.z;
		w -= val.w;
	}

	constexpr Vector4I Vector4I::operator*(const Vector4I &val) const
	{
		return {x * val.x, y * val.y, z * val.z, w * val.w};
	}

	constexpr Vector4I Vector4I::operator*(Int val) const
	{
		return {x * val, y * val, z * val, w * val};
	}

	constexpr void Vector4I::operator*=(Int val)
	{
		x *= val;
		y *= val;
		z *= val;
		w *= val;
	}

	constexpr Vector4I Vector4I::operator/(const Vector4I &val) const
	{
		return {x / val.x, y / val.y, z / val.z, w / val.w};
	}

	constexpr Vector4I Vector4I::operator/(Int val) const
	{
		return {x / val, y / val, z / val, w / val};
	}

	constexpr void Vector4I::operator/=(Int val)
	{
		x /= val;
		y /= val;
		z /= val;
		w /= val;
	}

	constexpr Vector4I Vector4I::operator-() const
	{
		return {-x, -y, -z - w};
	}

	constexpr bool Vector4I::operator==(const Vector4I &val) const
	{
		return x == val.x && y == val.y && z == val.z && w == val.w;
	}

	constexpr bool Vector4I::operator!=(const Vector4I &val) const
	{
		return x != val.x || y != val.y && z != val.z || w != val.w;
	}

	Vector4I::operator String() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]"};
	}

	String Vector4I::to_string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + "]"};
	}

	Float Vector4I::length() const
	{
		return math::sqrt(static_cast<float>(x * x + y * y + z * z + w * w));
	}

	Float Vector4I::dot(const Vector4I &v) const
	{
		return x * v.x + y * v.y + z * v.z + w * w;
	}

	Float Vector4I::angle(const Vector4I &v) const
	{
		return math::acos(dot(v));
	}
}
