#include "math/vector2i.hpp"

#include "vector2.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	constexpr Vector2I Vector2I::operator+(Vector2I val) const
	{
		return {x + val.x, y + val.y};
	}

	constexpr void Vector2I::operator+=(const Vector2I &val)
	{
		x += val.x;
		y += val.y;
	}

	constexpr Vector2I Vector2I::operator-(const Vector2I &val) const
	{
		return {x - val.x, y - val.y};
	}

	constexpr void Vector2I::operator-=(const Vector2I &val)
	{
		x -= val.x;
		y -= val.y;
	}

	constexpr Vector2I Vector2I::operator*(const Vector2I &val) const
	{
		return {x * val.x, y * val.y};
	}

	constexpr Vector2I Vector2I::operator*(Int val) const
	{
		return {x * val, y * val};
	}

	constexpr void Vector2I::operator*=(Int val)
	{
		x *= val;
		y *= val;
	}

	constexpr Vector2I Vector2I::operator/(const Vector2I &val) const
	{
		return {x / val.x, y / val.y};
	}

	constexpr Vector2I Vector2I::operator/(Int val) const
	{
		return {x / val, y / val};
	}

	constexpr void Vector2I::operator/=(Int val)
	{
		x /= val;
		y /= val;
	}

	constexpr Vector2I Vector2I::operator-() const
	{
		return {-x, -y};
	}

	constexpr bool Vector2I::operator==(const Vector2I &val) const
	{
		return x == val.x && y == val.y;
	}

	constexpr bool Vector2I::operator!=(const Vector2I &val) const
	{
		return x != val.x || y != val.y;
	}

	Vector2I::operator String() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + "]"};
	}

	String Vector2I::to_string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + "]"};
	}

	float32 Vector2I::length() const
	{
		return math::sqrt(static_cast<float32>(x * x + y * y));
	}

	float32 Vector2I::dot(const Vector2I &v) const
	{
		return x * v.x + y * v.y;
	}

	float32 Vector2I::angle(const Vector2I &v) const
	{
		return math::acos(dot(v));
	}

	Vector2I::operator Vector2() const
	{
		return Vector2{static_cast<float32>(x), static_cast<float32>(y)};
	}
}
