#include "math/vector2.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
	constexpr Vector2 Vector2::operator+(Vector2 val) const
	{
		return {x + val.x, y + val.y};
	}

	constexpr void Vector2::operator+=(const Vector2 &val)
	{
		x += val.x;
		y += val.y;
	}

	constexpr Vector2 Vector2::operator-(const Vector2 &val) const
	{
		return {x - val.x, y - val.y};
	}

	constexpr void Vector2::operator-=(const Vector2 &val)
	{
		x -= val.x;
		y -= val.y;
	}

	constexpr Vector2 Vector2::operator*(const Vector2 &val) const
	{
		return {x * val.x, y * val.y};
	}

	constexpr Vector2 Vector2::operator*(Float val) const
	{
		return {x * val, y * val};
	}

	constexpr void Vector2::operator*=(Float val)
	{
		x *= val;
		y *= val;
	}

	constexpr Vector2 Vector2::operator/(const Vector2 &val) const
	{
		return {x / val.x, y / val.y};
	}

	constexpr Vector2 Vector2::operator/(Float val) const
	{
		return {x / val, y / val};
	}

	constexpr void Vector2::operator/=(Float val)
	{
		x /= val;
		y /= val;
	}

	constexpr Vector2 Vector2::operator-() const
	{
		return {-x, -y};
	}

	constexpr bool Vector2::operator==(const Vector2 &val) const
	{
		return x == val.x && y == val.y;
	}

	constexpr bool Vector2::operator!=(const Vector2 &val) const
	{
		return x != val.x || y != val.y;
	}

	Vector2::operator std::string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + "]"};
	}

	std::string Vector2::to_string() const
	{
		return {"[" + std::to_string(x) + ", " + std::to_string(y) + "]"};
	}

	void Vector2::normalize()
	{
		x /= length();
		y /= length();
	}

	bool Vector2::isNormalized() const
	{
		return length() == 1.0f;
	}

	Vector2 Vector2::normalized() const
	{
		return {x / length(), y / length()};
	}

	Float Vector2::length() const
	{
		return math::sqrt(x * x + y * y);
	}

	Float Vector2::dot(const Vector2 &v) const
	{
		return x * v.x + y * v.y;
	}

	Float Vector2::angle(const Vector2 &v) const
	{
		return math::acos(dot(v));
	}
}
