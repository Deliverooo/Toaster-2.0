#include "vector2i.hpp"

#include "vector2.hpp"
#include "math/math_funcs.hpp"

namespace tst
{
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
		return static_cast<float32>(x * v.x + y * v.y);
	}

	float32 Vector2I::angle(const Vector2I &v) const
	{
		return math::acos(dot(v));
	}

	Vector2I Vector2I::clamp(const Vector2I &min, const Vector2I &max) const
	{
		return Vector2I{math::clamp(x, min.x, max.x), math::clamp(y, min.y, max.y)};
	}

	Vector2I Vector2I::clamp(int32 min, int32 max) const
	{
		return Vector2I{math::clamp(x, min, max), math::clamp(y, min, max)};
	}

	Vector2I Vector2I::min(const Vector2I &v) const
	{
		return {math::min(x, v.x), math::min(y, v.y)};
	}

	Vector2I Vector2I::max(const Vector2I &v) const
	{
		return {math::max(x, v.x), math::max(y, v.y)};
	}

	Vector2I::operator Vector2() const
	{
		return Vector2{static_cast<float32>(x), static_cast<float32>(y)};
	}
}
