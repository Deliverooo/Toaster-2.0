#pragma once

#include "vec3.hpp"

namespace tsm
{
	template<typename Type>
	struct Vec4
	{
		static constexpr u32 dim{4u};

		static const Vec4 zero;
		static const Vec4 one;

		static const Vec4 unitX;
		static const Vec4 unitY;
		static const Vec4 unitZ;
		static const Vec4 unitW;

		union
		{
			Type data[dim];

			struct
			{
				Type x;
				Type y;
				Type z;
				Type w;
			};
		};

		constexpr Vec4() = default;

		constexpr Vec4(Type p_x, Type p_y, Type p_z, Type p_w) : x(p_x), y(p_y), z(p_z), w(p_w)
		{
		};

		constexpr Vec4(Type p_s) : x(p_s), y(p_s), z(p_s), w(p_s)
		{
		}

		constexpr Vec4(Type p_x, const Vec3<Type> &p_v) : x(p_x), y(p_v.x), z(p_v.y), w(p_v.z)
		{
		};

		constexpr Vec4(const Vec3<Type> &p_v, Type p_w) : x(p_v.x), y(p_v.y), z(p_v.z), w(p_w)
		{
		};

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
		constexpr auto dot(const Vec4 &p_other) const -> Type { return tsm::dot(*this, p_other); }

		constexpr auto operator ==(Vec4 p_other) const -> bool
		{
			return x == p_other.x && y == p_other.y && z == p_other.z && w == p_other.w;
		}

		constexpr auto operator !=(Vec4 p_other) const -> bool
		{
			return x != p_other.x || y != p_other.y || z != p_other.z || w != p_other.w;
		}

		// Unary
		constexpr auto operator+(const Vec4 &p_v) -> Vec4
		{
			return p_v;
		}

		constexpr auto operator-(const Vec4 &p_v) -> Vec4
		{
			return Vec4{-p_v.x, -p_v.y, -p_v.z, -p_v.w};
		}

		constexpr operator Vec2<Type>()
		{
			return Vec2{x, y};
		}

		constexpr operator Vec3<Type>()
		{
			return Vec3{x, y, z};
		}

		constexpr auto operator+=(Type p_s) -> Vec4 &
		{
			x += p_s;
			y += p_s;
			z += p_s;
			w += p_s;
			return *this;
		}

		constexpr auto operator-=(Type p_s) -> Vec4 &
		{
			x -= p_s;
			y -= p_s;
			z -= p_s;
			w -= p_s;
			return *this;
		}

		constexpr auto operator+=(Vec4 p_v) -> Vec4 &
		{
			x += p_v.x;
			y += p_v.y;
			z += p_v.z;
			w += p_v.w;
			return *this;
		}

		constexpr auto operator-=(Vec4 p_v) -> Vec4 &
		{
			x -= p_v.x;
			y -= p_v.y;
			z -= p_v.z;
			w -= p_v.w;
			return *this;
		}

		constexpr auto operator*=(Type p_s) -> Vec4 &
		{
			x *= p_s;
			y *= p_s;
			z *= p_s;
			w *= p_s;
			return *this;
		}

		constexpr auto operator/=(Type p_s) -> Vec4 &
		{
			x /= p_s;
			y /= p_s;
			z /= p_s;
			w /= p_s;
			return *this;
		}

		constexpr auto operator*=(Vec4 p_v) -> Vec4 &
		{
			x *= p_v.x;
			y *= p_v.y;
			z *= p_v.z;
			w *= p_v.w;
			return *this;
		}

		constexpr auto operator/=(Vec4 p_v) -> Vec4 &
		{
			x /= p_v.x;
			y /= p_v.y;
			z /= p_v.z;
			w /= p_v.w;
			return *this;
		}

		template<typename TOther>
		constexpr operator Vec4<TOther>() const
		{
			return Vec4<TOther>{static_cast<TOther>(x), static_cast<TOther>(y), static_cast<TOther>(z), static_cast<TOther>(w)};
		}
	};

	template<typename Type>
	constexpr Vec4<Type> Vec4<Type>::zero{0};
	template<typename Type>
	constexpr Vec4<Type> Vec4<Type>::one{1};

	template<typename Type>
	constexpr Vec4<Type> Vec4<Type>::unitX{1, 0, 0, 0};
	template<typename Type>
	constexpr Vec4<Type> Vec4<Type>::unitY{0, 1, 0, 0};
	template<typename Type>
	constexpr Vec4<Type> Vec4<Type>::unitZ{0, 0, 1, 0};
	template<typename Type>
	constexpr Vec4<Type> Vec4<Type>::unitW{0, 0, 0, 1};

	// Binary
	template<typename Type>
	constexpr auto operator+(const Vec4<Type> &p_v, Type p_s) -> Vec4<Type> { return Vec4{p_v.x + p_s, p_v.y + p_s, p_v.z + p_s, p_v.w + p_s}; }

	template<typename Type>
	constexpr auto operator+(Type p_s, const Vec4<Type> &p_v) -> Vec4<Type> { return Vec4{p_s + p_v.x, p_s + p_v.y, p_s + p_v.z, p_s + p_v.w}; }

	template<typename Type>
	constexpr auto operator+(const Vec4<Type> &p_v1, const Vec4<Type> &p_v2) -> Vec4<Type>
	{
		return Vec4{p_v1.x + p_v2.x, p_v1.y + p_v2.y, p_v1.z + p_v2.z, p_v1.w + p_v2.w};
	}

	template<typename Type>
	constexpr auto operator-(const Vec4<Type> &p_v, Type p_s) -> Vec4<Type> { return Vec4{p_v.x - p_s, p_v.y - p_s, p_v.z - p_s, p_v.w - p_s}; }

	template<typename Type>
	constexpr auto operator-(Type p_s, const Vec4<Type> &p_v) -> Vec4<Type> { return Vec4{p_s - p_v.x, p_s - p_v.y, p_s - p_v.z, p_s - p_v.w}; }

	template<typename Type>
	constexpr auto operator-(const Vec4<Type> &p_v1, const Vec4<Type> &p_v2) -> Vec4<Type>
	{
		return Vec4{p_v1.x - p_v2.x, p_v1.y - p_v2.y, p_v1.z - p_v2.z, p_v1.w - p_v2.w};
	}

	template<typename Type>
	constexpr auto operator*(const Vec4<Type> &p_v, Type p_s) -> Vec4<Type> { return Vec4{p_v.x * p_s, p_v.y * p_s, p_v.z * p_s, p_v.w * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const Vec4<Type> &p_v) -> Vec4<Type> { return Vec4{p_s * p_v.x, p_s * p_v.y, p_s * p_v.z, p_s * p_v.w}; }

	template<typename Type>
	constexpr auto operator/(const Vec4<Type> &p_v, Type p_s) -> Vec4<Type> { return Vec4{p_v.x / p_s, p_v.y / p_s, p_v.z / p_s, p_v.w / p_s}; }

	template<typename Type>
	constexpr auto operator/(Type p_s, const Vec4<Type> &p_v) -> Vec4<Type> { return Vec4{p_s / p_v.x, p_s / p_v.y, p_s / p_v.z, p_s / p_v.w}; }

	template<typename Type>
	constexpr auto operator*(const Vec4<Type> &p_v1, const Vec4<Type> &p_v2) -> Vec4<Type>
	{
		return Vec4{p_v1.x * p_v2.x, p_v1.y * p_v2.y, p_v1.z * p_v2.z, p_v1.w * p_v2.w};
	}

	template<typename Type>
	constexpr auto operator/(const Vec4<Type> &p_v1, const Vec4<Type> &p_v2) -> Vec4<Type>
	{
		return Vec4{p_v1.x / p_v2.x, p_v1.y / p_v2.y, p_v1.z / p_v2.z, p_v1.w / p_v2.w};
	}

	template<typename Type>
	constexpr Vec3<Type>::Vec3(const Vec4<Type> &p_v) : x(p_v.x), y(p_v.y), z(p_v.z)
	{
	}

	template<typename Type>
	constexpr auto length(const Vec4<Type> &p_v) -> Type
	{
		return static_cast<Type>(std::sqrt(p_v.x * p_v.x + p_v.y * p_v.y + p_v.z * p_v.z + p_v.w * p_v.w));
	}

	template<typename Type>
	constexpr auto normalize(const Vec4<Type> &p_v) -> Vec4<Type>
	{
		Type l{length(p_v)};
		return Vec4{p_v.x / l, p_v.y / l, p_v.z / l, p_v.w / l};
	}

	template<typename Type>
	constexpr auto dot(const Vec4<Type> &p_v1, const Vec4<Type> &p_v2) -> Type
	{
		return p_v1.x * p_v2.x + p_v1.y * p_v2.y + p_v1.z * p_v2.z + p_v1.w * p_v2.w;
	}
}
