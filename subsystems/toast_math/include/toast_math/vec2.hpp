#pragma once

#include <cmath>
#include "types.hpp"

namespace tsm
{
	template<typename Type>
	struct Vec2;
	template<typename Type>
	struct Vec3;
	template<typename Type>
	struct Vec4;

	template<typename Type>
	constexpr auto length(Vec2<Type> p_v) -> Type;
	template<typename Type>
	constexpr auto length(const Vec3<Type> &p_v) -> Type;
	template<typename Type>
	constexpr auto length(const Vec4<Type> &p_v) -> Type;

	template<typename Type>
	constexpr auto normalize(Vec2<Type> p_v) -> Vec2<Type>;
	template<typename Type>
	constexpr auto normalize(const Vec3<Type> &p_v) -> Vec3<Type>;
	template<typename Type>
	constexpr auto normalize(const Vec4<Type> &p_v) -> Vec4<Type>;

	template<typename Type>
	constexpr auto dot(Vec2<Type> p_v1, Vec2<Type> p_v2) -> Type;
	template<typename Type>
	constexpr auto dot(const Vec3<Type> &p_v1, const Vec3<Type> &p_v2) -> Type;
	template<typename Type>
	constexpr auto dot(const Vec4<Type> &p_v1, const Vec4<Type> &p_v2) -> Type;

	template<typename Type>
	struct Vec2
	{
		using DefaultFunctionType = f32;

		static constexpr u32 dim{2u};

		static const Vec2 zero;
		static const Vec2 one;

		static const Vec2 unitX;
		static const Vec2 unitY;

		union
		{
			Type data[dim];

			struct
			{
				Type x;
				Type y;
			};
		};

		constexpr Vec2() = default;

		constexpr Vec2(Type p_x, Type p_y) : x(p_x), y(p_y)
		{
		};

		constexpr Vec2(Type p_s) : x(p_s), y(p_s)
		{
		}

		constexpr Type &operator[](i32 p_index)
		{
			TSM_ASSERT_LENGTH(p_index, dim);
			return data[p_index];
		}

		constexpr auto operator[](i32 p_index) const -> const Type &
		{
			TSM_ASSERT_LENGTH(p_index, dim);
			return data[p_index];
		}

		constexpr auto length() const -> Type { return tsm::length(*this); }

		constexpr auto dot(Vec2 p_other) const -> Type { return tsm::dot(*this, p_other); }

		// Returns the aspect of the vector (useful if the vector represents a window size)
		constexpr auto aspect() const -> DefaultFunctionType { return static_cast<DefaultFunctionType>(x) / static_cast<DefaultFunctionType>(y); }

		constexpr auto operator ==(Vec2 p_other) const -> bool
		{
			return x == p_other.x && y == p_other.y;
		}

		constexpr auto operator !=(Vec2 p_other) const -> bool
		{
			return x != p_other.x || y != p_other.y;
		}

		// Unary
		constexpr auto operator+() const -> Vec2 { return *this; }
		constexpr auto operator-() const -> Vec2 { return Vec2{-x, -y}; }

		// X= operators
		constexpr auto operator+=(Type p_s) -> Vec2 &
		{
			x += p_s;
			y += p_s;
			return *this;
		}

		constexpr auto operator-=(Type p_s) -> Vec2 &
		{
			x -= p_s;
			y -= p_s;
			return *this;
		}

		constexpr auto operator+=(Vec2 p_v) -> Vec2 &
		{
			x += p_v.x;
			y += p_v.y;
			return *this;
		}

		constexpr auto operator-=(Vec2 p_v) -> Vec2 &
		{
			x -= p_v.x;
			y -= p_v.y;
			return *this;
		}

		constexpr auto operator*=(Type p_s) -> Vec2 &
		{
			x *= p_s;
			y *= p_s;
			return *this;
		}

		constexpr auto operator/=(Type p_s) -> Vec2 &
		{
			x /= p_s;
			y /= p_s;
			return *this;
		}

		constexpr auto operator*=(Vec2 p_v) -> Vec2 &
		{
			x *= p_v.x;
			y *= p_v.y;
			return *this;
		}

		constexpr auto operator/=(Vec2 p_v) -> Vec2 &
		{
			x /= p_v.x;
			y /= p_v.y;
			return *this;
		}

		template<typename TOther>
		constexpr operator Vec2<TOther>() const
		{
			return Vec2<TOther>{static_cast<TOther>(x), static_cast<TOther>(y)};
		}
	};

	template<typename Type>
	constexpr Vec2<Type> Vec2<Type>::zero{0};
	template<typename Type>
	constexpr Vec2<Type> Vec2<Type>::one{1};

	template<typename Type>
	constexpr Vec2<Type> Vec2<Type>::unitX{1, 0};
	template<typename Type>
	constexpr Vec2<Type> Vec2<Type>::unitY{0, 1};

	// Binary
	template<typename Type>
	constexpr auto operator+(const Vec2<Type> &p_v, Type p_s) -> Vec2<Type> { return Vec2{p_v.x + p_s, p_v.y + p_s}; }

	template<typename Type>
	constexpr auto operator+(Type p_s, const Vec2<Type> &p_v) -> Vec2<Type> { return Vec2{p_s + p_v.x, p_s + p_v.y}; }

	template<typename Type>
	constexpr auto operator+(const Vec2<Type> &p_v1, const Vec2<Type> &p_v2) -> Vec2<Type> { return Vec2{p_v1.x + p_v2.x, p_v1.y + p_v2.y}; }

	template<typename Type>
	constexpr auto operator-(const Vec2<Type> &p_v, Type p_s) -> Vec2<Type> { return Vec2{p_v.x - p_s, p_v.y - p_s}; }

	template<typename Type>
	constexpr auto operator-(Type p_s, const Vec2<Type> &p_v) -> Vec2<Type> { return Vec2{p_s - p_v.x, p_s - p_v.y}; }

	template<typename Type>
	constexpr auto operator-(const Vec2<Type> &p_v1, const Vec2<Type> &p_v2) -> Vec2<Type> { return Vec2{p_v1.x - p_v2.x, p_v1.y - p_v2.y}; }

	template<typename Type>
	constexpr auto operator*(const Vec2<Type> &p_v, Type p_s) -> Vec2<Type> { return Vec2{p_v.x * p_s, p_v.y * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const Vec2<Type> &p_v) -> Vec2<Type> { return Vec2{p_s * p_v.x, p_s * p_v.y}; }

	template<typename Type>
	constexpr auto operator/(const Vec2<Type> &p_v, Type p_s) -> Vec2<Type> { return Vec2{p_v.x / p_s, p_v.y / p_s}; }

	template<typename Type>
	constexpr auto operator/(Type p_s, const Vec2<Type> &p_v) -> Vec2<Type> { return Vec2{p_s / p_v.x, p_s / p_v.y}; }

	template<typename Type>
	constexpr auto operator*(const Vec2<Type> &p_v1, const Vec2<Type> &p_v2) -> Vec2<Type>
	{
		return Vec2{p_v1.x * p_v2.x, p_v1.y * p_v2.y, p_v1.z * p_v2.z};
	}

	template<typename Type>
	constexpr auto operator/(const Vec2<Type> &p_v1, const Vec2<Type> &p_v2) -> Vec2<Type>
	{
		return Vec2{p_v1.x / p_v2.x, p_v1.y / p_v2.y};
	}

	template<typename Type>
	constexpr auto length(Vec2<Type> p_v) -> Type
	{
		return static_cast<Type>(std::sqrt(p_v.x * p_v.x + p_v.y * p_v.y));
	}

	template<typename Type>
	constexpr auto normalize(Vec2<Type> p_v) -> Vec2<Type>
	{
		Type l{length(p_v)};
		return Vec2{p_v.x / l, p_v.y / l};
	}

	template<typename Type>
	constexpr auto dot(Vec2<Type> p_v1, Vec2<Type> p_v2) -> Type
	{
		return p_v1.x * p_v2.x + p_v1.y * p_v2.y;
	}
}
