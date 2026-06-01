#pragma once

#include "vec2.hpp"

namespace tsm
{
	template<typename Type>
	struct Vec3
	{
		static constexpr u32 dim{3u};

		static const Vec3 zero;
		static const Vec3 one;

		static const Vec3 unitX;
		static const Vec3 unitY;
		static const Vec3 unitZ;

		union
		{
			Type data[dim];

			struct
			{
				Type x;
				Type y;
				Type z;
			};
		};

		constexpr Vec3() = default;

		constexpr Vec3(Type p_x, Type p_y, Type p_z) : x(p_x), y(p_y), z(p_z)
		{
		};

		constexpr Vec3(Type p_s) : x(p_s), y(p_s), z(p_s)
		{
		}

		constexpr Vec3(Vec2<Type> p_v, Type p_z) : x(p_v.x), y(p_v.y), z(p_z)
		{
		}

		constexpr Vec3(Type p_x, Vec2<Type> p_v) : x(p_x), y(p_v.y), z(p_v.z)
		{
		}

		constexpr Vec3(const Vec4<Type> &p_v);

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
		constexpr auto dot(const Vec3 &p_other) const -> Type { return tsm::dot(*this, p_other); }

		constexpr auto operator ==(Vec3 p_other) const -> bool
		{
			return x == p_other.x && y == p_other.y && z == p_other.z;
		}

		constexpr auto operator !=(Vec3 p_other) const -> bool
		{
			return x != p_other.x || y != p_other.y || z != p_other.z;
		}

		// Unary
		constexpr auto operator+(const Vec3 &p_v) -> Vec3
		{
			return p_v;
		}

		constexpr auto operator-(const Vec3 &p_v) -> Vec3
		{
			return Vec3{-p_v.x, -p_v.y, -p_v.z};
		}

		constexpr operator Vec2<Type>()
		{
			return Vec2{x, y};
		}

		constexpr auto operator+=(Type p_s) -> Vec3 &
		{
			x += p_s;
			y += p_s;
			z += p_s;
			return *this;
		}

		constexpr auto operator-=(Type p_s) -> Vec3 &
		{
			x -= p_s;
			y -= p_s;
			z -= p_s;
			return *this;
		}

		constexpr auto operator+=(Vec3 p_v) -> Vec3 &
		{
			x += p_v.x;
			y += p_v.y;
			z += p_v.z;
			return *this;
		}

		constexpr auto operator-=(Vec3 p_v) -> Vec3 &
		{
			x -= p_v.x;
			y -= p_v.y;
			z -= p_v.z;
			return *this;
		}

		constexpr auto operator*=(Type p_s) -> Vec3 &
		{
			x *= p_s;
			y *= p_s;
			z *= p_s;
			return *this;
		}

		constexpr auto operator/=(Type p_s) -> Vec3 &
		{
			x /= p_s;
			y /= p_s;
			z /= p_s;
			return *this;
		}

		constexpr auto operator*=(Vec3 p_v) -> Vec3 &
		{
			x *= p_v.x;
			y *= p_v.y;
			z *= p_v.z;
			return *this;
		}

		constexpr auto operator/=(Vec3 p_v) -> Vec3 &
		{
			x /= p_v.x;
			y /= p_v.y;
			z /= p_v.z;
			return *this;
		}

		template<typename TOther>
		constexpr operator Vec3<TOther>() const
		{
			return Vec3<TOther>{static_cast<TOther>(x), static_cast<TOther>(y), static_cast<TOther>(z)};
		}
	};

	template<typename Type>
	constexpr Vec3<Type> Vec3<Type>::zero{0};
	template<typename Type>
	constexpr Vec3<Type> Vec3<Type>::one{1};

	template<typename Type>
	constexpr Vec3<Type> Vec3<Type>::unitX{1, 0, 0};
	template<typename Type>
	constexpr Vec3<Type> Vec3<Type>::unitY{0, 1, 0};
	template<typename Type>
	constexpr Vec3<Type> Vec3<Type>::unitZ{0, 0, 1};

	// Binary
	template<typename Type>
	constexpr auto operator+(const Vec3<Type> &p_v, Type p_s) -> Vec3<Type> { return Vec3{p_v.x + p_s, p_v.y + p_s, p_v.z + p_s}; }

	template<typename Type>
	constexpr auto operator+(Type p_s, const Vec3<Type> &p_v) -> Vec3<Type> { return Vec3{p_s + p_v.x, p_s + p_v.y, p_s + p_v.z}; }

	template<typename Type>
	constexpr auto operator+(const Vec3<Type> &p_v1, const Vec3<Type> &p_v2) -> Vec3<Type> { return Vec3{p_v1.x + p_v2.x, p_v1.y + p_v2.y, p_v1.z + p_v2.z}; }

	template<typename Type>
	constexpr auto operator-(const Vec3<Type> &p_v, Type p_s) -> Vec3<Type> { return Vec3{p_v.x - p_s, p_v.y - p_s, p_v.z - p_s}; }

	template<typename Type>
	constexpr auto operator-(Type p_s, const Vec3<Type> &p_v) -> Vec3<Type> { return Vec3{p_s - p_v.x, p_s - p_v.y, p_s - p_v.z}; }

	template<typename Type>
	constexpr auto operator-(const Vec3<Type> &p_v1, const Vec3<Type> &p_v2) -> Vec3<Type> { return Vec3{p_v1.x - p_v2.x, p_v1.y - p_v2.y, p_v1.z - p_v2.z}; }

	template<typename Type>
	constexpr auto operator*(const Vec3<Type> &p_v, Type p_s) -> Vec3<Type> { return Vec3{p_v.x * p_s, p_v.y * p_s, p_v.z * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const Vec3<Type> &p_v) -> Vec3<Type> { return Vec3{p_s * p_v.x, p_s * p_v.y, p_s * p_v.z}; }

	template<typename Type>
	constexpr auto operator/(const Vec3<Type> &p_v, Type p_s) -> Vec3<Type> { return Vec3{p_v.x / p_s, p_v.y / p_s, p_v.z / p_s}; }

	template<typename Type>
	constexpr auto operator/(Type p_s, const Vec3<Type> &p_v) -> Vec3<Type> { return Vec3{p_s / p_v.x, p_s / p_v.y, p_s / p_v.z}; }

	template<typename Type>
	constexpr auto operator*(const Vec3<Type> &p_v1, const Vec3<Type> &p_v2) -> Vec3<Type>
	{
		return Vec3{p_v1.x * p_v2.x, p_v1.y * p_v2.y, p_v1.z * p_v2.z};
	}

	template<typename Type>
	constexpr auto operator/(const Vec3<Type> &p_v1, const Vec3<Type> &p_v2) -> Vec3<Type>
	{
		return Vec3{p_v1.x / p_v2.x, p_v1.y / p_v2.y, p_v1.z / p_v2.z};
	}

	template<typename Type>
	constexpr auto length(const Vec3<Type> &p_v) -> Type
	{
		return static_cast<Type>(std::sqrt(p_v.x * p_v.x + p_v.y * p_v.y + p_v.z * p_v.z));
	}

	template<typename Type>
	constexpr auto normalize(const Vec3<Type> &p_v) -> Vec3<Type>
	{
		Type l{length(p_v)};
		return Vec3{p_v.x / l, p_v.y / l, p_v.z / l};
	}

	template<typename Type>
	constexpr auto dot(const Vec3<Type> &p_v1, const Vec3<Type> &p_v2) -> Type
	{
		return p_v1.x * p_v2.x + p_v1.y * p_v2.y + p_v1.z * p_v2.z;
	}
}
