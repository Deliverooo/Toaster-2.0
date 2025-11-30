#include "math/vector2.hpp"
#include "math/vector2i.hpp"
#include "math/math_funcs.hpp"

namespace tst
{


	String Vector2::to_string() const
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

	float32 Vector2::length() const
	{
		return math::sqrt(x * x + y * y);
	}

	float32 Vector2::dot(const Vector2 &v) const
	{
		return x * v.x + y * v.y;
	}

	float32 Vector2::angle(const Vector2 &v) const
	{
		return math::acos(dot(v));
	}

	Vector2 Vector2::min(const Vector2 &v) const
	{
		return {math::min(x, v.x), math::min(y, v.y)};
	}

	Vector2 Vector2::max(const Vector2 &v) const
	{
		return {math::max(x, v.x), math::max(y, v.y)};
	}

	Vector2::operator Vector2I() const
	{
		return {static_cast<int32>(x), static_cast<int32>(y)};
	}
}
