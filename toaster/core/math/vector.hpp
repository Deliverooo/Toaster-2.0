/*
*  	Copyright 2025 Orbo Stetson
 *  	Licensed under the Apache License, Version 2.0 (the "License");
 *  	you may not use this file except in compliance with the License.
 *  	You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 *    	Unless required by applicable law or agreed to in writing, software
 *    	distributed under the License is distributed on an "AS IS" BASIS,
 *    	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    	See the License for the specific language governing permissions and
 *    	limitations under the License.
 */
#pragma once
#include "math_constants.hpp"
#include "core_typedefs.hpp"
#include "error_macros.hpp"

#undef min
#undef max

#include <cmath>

namespace tsm
{
	#define VECTOR_MEMBERS(T, n) \
	/* Subscript operators - built-in subscripts are ambiguous without these */ \
	T & operator [] (int i) \
	{ return data()[i]; } \
	const T & operator [] (int i) const \
	{ return data()[i]; } \
	static constexpr uint8 const DIM = n; \
	vec() { } \
	vec(const T* v) \
	{ for(int i = 0; i < n; i++) data()[i] = v[i]; } \
	private: operator bool();

	template<typename Type, int NDim>
	struct vec
	{
		static_assert(NDim > 4);

		Type m_data[NDim];

		vec(Type a)
		{
			for (int i    = 0; i < NDim; i++)
				m_data[i] = a;
		}

		Type *      data() { return m_data; }
		const Type *data() const { return m_data; }

		template<typename U>
		explicit vec(const vec<U, NDim> &v)
		{
			for (int i    = 0; i < NDim; i++)
				m_data[i] = static_cast<Type>(v.m_data[i]);
		}

		VECTOR_MEMBERS(Type, NDim)
	};

	template<typename T>
	struct vec<T, 2>
	{
		T x, y;

		T *                    data() { return &x; }
		[[nodiscard]] const T *data() const { return &x; }

		constexpr vec(T a) : x(a), y(a)
		{
		}

		constexpr vec(T _x, T _y) : x(_x), y(_y)
		{
		}

		template<typename U>
		explicit constexpr vec(const vec<U, 2> &v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y))
		{
		}

		constexpr vec(const vec<T, 3> &v) : x(v.x), y(v.y)
		{
		}

		constexpr vec(const vec<T, 4> &v) : x(v.x), y(v.y)
		{
		}

		constexpr static vec zero() { return vec(static_cast<T>(0)); }

		[[nodiscard]] constexpr float32 aspect() const { return static_cast<float32>(x) / static_cast<float32>(x); }

		VECTOR_MEMBERS(T, 2)
	};

	template<typename T>
	struct vec<T, 3>
	{
		T x, y, z;

		T *      data() { return &x; }
		const T *data() const { return &x; }

		vec<T, 2> &      xy() { return *reinterpret_cast<vec<T, 2> *>(&x); }
		const vec<T, 2> &xy() const { return *reinterpret_cast<const vec<T, 2> *>(&x); }

		constexpr vec(T a) : x(a), y(a), z(a)
		{
		}

		constexpr vec(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
		{
		}

		constexpr vec(const vec<T, 2> &xy, T _z) : x(xy.x), y(xy.y), z(_z)
		{
		}

		constexpr vec(const vec<T, 4> &v) : x(v.x), y(v.y), z(v.z)
		{
		}

		template<typename U>
		explicit constexpr vec(const vec<U, 3> &v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z))
		{
		}

		constexpr static vec zero() { return vec(static_cast<T>(0)); }

		VECTOR_MEMBERS(T, 3)
	};

	template<typename T>
	struct vec<T, 4>
	{
		T x, y, z, w;

		T *      data() { return &x; }
		const T *data() const { return &x; }

		vec<T, 2> &      xy() { return *reinterpret_cast<vec<T, 2> *>(&x); }
		const vec<T, 2> &xy() const { return *reinterpret_cast<const vec<T, 2> *>(&x); }
		vec<T, 2> &      zw() { return *reinterpret_cast<vec<T, 2> *>(&z); }
		const vec<T, 2> &zw() const { return *reinterpret_cast<const vec<T, 2> *>(&z); }
		vec<T, 3> &      xyz() { return *reinterpret_cast<vec<T, 3> *>(&x); }
		const vec<T, 3> &xyz() const { return *reinterpret_cast<const vec<T, 3> *>(&x); }

		constexpr vec(T a) : x(a), y(a), z(a), w(a)
		{
		}

		constexpr vec(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w)
		{
		}

		constexpr vec(const vec<T, 2> &xy, T _z, T _w) : x(xy.x), y(xy.y), z(_z), w(_w)
		{
		}

		constexpr vec(const vec<T, 2> &xy, const vec<T, 2> &zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
		{
		}

		constexpr vec(const vec<T, 3> &xyz, T _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w)
		{
		}

		template<typename U>
		explicit constexpr vec(const vec<U, 4> &v) : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z)), w(static_cast<T>(v.w))
		{
		}

		constexpr static vec zero() { return vec(static_cast<T>(0)); }

		VECTOR_MEMBERS(T, 4)
	};

	#undef VECTOR_MEMBERS

	using vec2f = vec<float32, 2>;
	using vec3f = vec<float32, 3>;
	using vec4f = vec<float32, 4>;

	using vec2d = vec<float64, 2>;
	using vec3d = vec<float64, 3>;
	using vec4d = vec<float64, 4>;

	using vec2i = vec<int32, 2>;
	using vec3i = vec<int32, 3>;
	using vec4i = vec<int32, 4>;

	using vec2ui = vec<uint32, 2>;
	using vec3ui = vec<uint32, 3>;
	using vec4ui = vec<uint32, 4>;

	using vec2b = vec<bool, 2>;
	using vec3b = vec<bool, 3>;
	using vec4b = vec<bool, 4>;

	#define DEFINE_UNARY_OPERATOR(op) \
        template<typename T> constexpr vec<T, 2> operator op (const vec<T, 2>& a) \
        { return vec<T, 2>(op a.x, op a.y); } \
        template<typename T> constexpr vec<T, 3> operator op (const vec<T, 3>& a) \
        { return vec<T, 3>(op a.x, op a.y, op a.z); } \
        template<typename T> constexpr vec<T, 4> operator op (const vec<T, 4>& a) \
        { return vec<T, 4>(op a.x, op a.y, op a.z, op a.w); }

	#define DEFINE_BINARY_OPERATORS(op) \
        /* vec-vec op */ \
        template<typename T> constexpr vec<T, 2> operator op (const vec<T, 2>& a, const vec<T, 2>& b) \
        { return vec<T, 2>(a.x op b.x, a.y op b.y); } \
        template<typename T> constexpr vec<T, 3> operator op (const vec<T, 3>& a, const vec<T, 3>& b) \
        { return vec<T, 3>(a.x op b.x, a.y op b.y, a.z op b.z); } \
        template<typename T> constexpr vec<T, 4> operator op (const vec<T, 4>& a, const vec<T, 4>& b) \
        { return vec<T, 4>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); } \
        /* Scalar-vec op */ \
        template<typename T> constexpr vec<T, 2> operator op (T a, const vec<T, 2>& b) \
        { return vec<T, 2>(a op b.x, a op b.y); } \
        template<typename T> constexpr vec<T, 3> operator op (T a, const vec<T, 3>& b) \
        { return vec<T, 3>(a op b.x, a op b.y, a op b.z); } \
        template<typename T> constexpr vec<T, 4> operator op (T a, const vec<T, 4>& b) \
        { return vec<T, 4>(a op b.x, a op b.y, a op b.z, a op b.w); } \
        /* vec-scalar op */ \
        template<typename T> constexpr vec<T, 2> operator op (const vec<T, 2>& a, T b) \
        { return vec<T, 2>(a.x op b, a.y op b); } \
        template<typename T> constexpr vec<T, 3> operator op (const vec<T, 3>& a, T b) \
        { return vec<T, 3>(a.x op b, a.y op b, a.z op b); } \
        template<typename T> constexpr vec<T, 4> operator op (const vec<T, 4>& a, T b) \
        { return vec<T, 4>(a.x op b, a.y op b, a.z op b, a.w op b); }

	#define DEFINE_INPLACE_OPERATORS(op) \
        /* vec-vec op */ \
        template<typename T> vec<T, 2> & operator op (vec<T, 2>& a, const vec<T, 2>& b) \
        { a.x op b.x; a.y op b.y; return a; } \
        template<typename T> vec<T, 3> & operator op (vec<T, 3>& a, const vec<T, 3>& b) \
        { a.x op b.x; a.y op b.y; a.z op b.z; return a; } \
        template<typename T> vec<T, 4> & operator op (vec<T, 4>& a, const vec<T, 4>& b) \
        { a.x op b.x; a.y op b.y; a.z op b.z; a.w op b.w; return a; } \
        /* vec-scalar op */ \
        template<typename T> vec<T, 2> & operator op (vec<T, 2>& a, T b) \
        { a.x op b; a.y op b; return a; } \
        template<typename T> vec<T, 3> & operator op (vec<T, 3>& a, T b) \
        { a.x op b; a.y op b; a.z op b; return a; } \
        template<typename T> vec<T, 4> & operator op (vec<T, 4>& a, T b) \
        { a.x op b; a.y op b; a.z op b; a.w op b; return a; }

	#define DEFINE_RELATIONAL_OPERATORS(op) \
        /* vec-vec op */ \
        template<typename T> constexpr vec<bool, 2> operator op (const vec<T, 2>& a, const vec<T, 2>& b) \
        { return vec<bool, 2>(a.x op b.x, a.y op b.y); } \
        template<typename T> constexpr vec<bool, 3> operator op (const vec<T, 3>& a, const vec<T, 3>& b) \
        { return vec<bool, 3>(a.x op b.x, a.y op b.y, a.z op b.z); } \
        template<typename T> constexpr vec<bool, 4> operator op (const vec<T, 4>& a, const vec<T, 4>& b) \
        { return vec<bool, 4>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); } \
        /* Scalar-vec op */ \
        template<typename T> constexpr vec<bool, 2> operator op (T a, const vec<T, 2>& b) \
        { return vec<bool, 2>(a op b.x, a op b.y); } \
        template<typename T> constexpr vec<bool, 3> operator op (T a, const vec<T, 3>& b) \
        { return vec<bool, 3>(a op b.x, a op b.y, a op b.z); } \
        template<typename T> constexpr vec<bool, 4> operator op (T a, const vec<T, 4>& b) \
        { return vec<bool, 4>(a op b.x, a op b.y, a op b.z, a op b.w); } \
        /* vec-scalar op */ \
        template<typename T> constexpr vec<bool, 2> operator op (const vec<T, 2>& a, T b) \
        { return vec<bool, 2>(a.x op b, a.y op b); } \
        template<typename T> constexpr vec<bool, 3> operator op (const vec<T, 3>& a, T b) \
        { return vec<bool, 3>(a.x op b, a.y op b, a.z op b); } \
        template<typename T> constexpr vec<bool, 4> operator op (const vec<T, 4>& a, T b) \
        { return vec<bool, 4>(a.x op b, a.y op b, a.z op b, a.w op b); }

	DEFINE_BINARY_OPERATORS(+);
	DEFINE_BINARY_OPERATORS(-);
	DEFINE_UNARY_OPERATOR(-);
	DEFINE_BINARY_OPERATORS(*);
	DEFINE_BINARY_OPERATORS(/);
	DEFINE_BINARY_OPERATORS(&);
	DEFINE_BINARY_OPERATORS(|);
	DEFINE_BINARY_OPERATORS(^);
	DEFINE_UNARY_OPERATOR(!);
	DEFINE_UNARY_OPERATOR(~);

	DEFINE_INPLACE_OPERATORS(+=);
	DEFINE_INPLACE_OPERATORS(-=);
	DEFINE_INPLACE_OPERATORS(*=);
	DEFINE_INPLACE_OPERATORS(/=);
	DEFINE_INPLACE_OPERATORS(&=);
	DEFINE_INPLACE_OPERATORS(|=);
	DEFINE_INPLACE_OPERATORS(^=);

	DEFINE_RELATIONAL_OPERATORS(==);
	DEFINE_RELATIONAL_OPERATORS(!=);
	DEFINE_RELATIONAL_OPERATORS(<);
	DEFINE_RELATIONAL_OPERATORS(>);
	DEFINE_RELATIONAL_OPERATORS(<=);
	DEFINE_RELATIONAL_OPERATORS(>=);

	#undef DEFINE_UNARY_OPERATOR
	#undef DEFINE_BINARY_OPERATORS
	#undef DEFINE_INPLACE_OPERATORS
	#undef DEFINE_RELATIONAL_OPERATORS

	// Other math functions

	template<typename T, int n>
	T dot(vec<T, n> const &a, vec<T, n> const &b)
	{
		T result(0);
		for (int i = 0; i < n; ++i)
			result += a[i] * b[i];
		return result;
	}

	template<typename T>
	constexpr T dot(vec<T, 2> const &a, vec<T, 2> const &b)
	{
		return a.x * b.x + a.y * b.y;
	}

	template<typename T>
	constexpr T dot(vec<T, 3> const &a, vec<T, 3> const &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	template<typename T>
	constexpr T dot(vec<T, 4> const &a, vec<T, 4> const &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	template<typename T, int n>
	T lengthSquared(vec<T, n> const &a)
	{
		return dot(a, a);
	}

	template<typename T, int n>
	T length(vec<T, n> const &a)
	{
		return sqrt(lengthSquared(a));
	}

	template<typename T, int n>
	vec<T, n> normalize(vec<T, n> const &a)
	{
		return a / length(a);
	}

	template<typename T, int n>
	vec<T, n> pow(vec<T, n> const &a, float p)
	{
		vec<T, n> result;
		for (int i    = 0; i < n; ++i)
			result[i] = ::pow(a[i], p);
		return result;
	}

	template<typename T, int n>
	vec<bool, n> isnear(vec<T, n> const &a, vec<T, n> const &b, float epsilon = constants::epsilon)
	{
		vec<bool, n> result;
		for (int i    = 0; i < n; ++i)
			result[i] = isnear(a[i], b[i], epsilon);
		return result;
	}

	template<typename T, int n>
	vec<bool, n> isnear(vec<T, n> const &a, T b, float epsilon = constants::epsilon)
	{
		vec<bool, n> result;
		for (int i    = 0; i < n; ++i)
			result[i] = isnear(a[i], b, epsilon);
		return result;
	}

	template<typename T, int n>
	vec<bool, n> isnear(T a, vec<T, n> const &b, float epsilon = constants::epsilon)
	{
		vec<bool, n> result;
		for (int i    = 0; i < n; ++i)
			result[i] = isnear(a, b[i], epsilon);
		return result;
	}

	template<typename T, int n>
	vec<bool, n> isfinite(vec<T, n> const &a)
	{
		vec<bool, n> result;
		for (int i    = 0; i < n; ++i)
			result[i] = isfinite(a[i]);
		return result;
	}

	template<typename T, int n>
	vec<int, n> round(vec<T, n> const &a)
	{
		vec<int, n> result;
		for (int i    = 0; i < n; ++i)
			result[i] = round(a[i]);
		return result;
	}

	template<typename T>
	constexpr vec<T, 3> cross(vec<T, 3> const &a, vec<T, 3> const &b)
	{
		return vec<T, 3>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
	}

	template<typename T>
	constexpr vec<T, 2> orthogonal(vec<T, 2> const &a)
	{
		return vec<T, 2>(-a.y, a.x);
	}

	template<typename T>
	constexpr vec<T, 3> orthogonal(vec<T, 3> const &a)
	{
		// Implementation due to Sam Hocevar - see blog post:
		// http://lolengine.net/blog/2013/09/21/picking-orthogonal-vec-combing-coconuts
		return (abs(a.x) > abs(a.z)) ? vec<T, 3>(-a.y, a.x, T(0)) : vec<T, 3>(T(0), -a.z, a.y);
	}

	// Utilities for bool vecs

	template<int n>
	bool any(vec<bool, n> const &a);

	template<>
	constexpr bool any(const vec<bool, 2> &a) { return a.x || a.y; }

	template<>
	constexpr bool any(const vec<bool, 3> &a) { return a.x || a.y || a.z; }

	template<>
	constexpr bool any(const vec<bool, 4> &a) { return a.x || a.y || a.z || a.w; }

	template<int n>
	bool all(vec<bool, n> const &a);

	template<>
	constexpr bool all(const vec<bool, 2> &a) { return a.x && a.y; }

	template<>
	constexpr bool all(const vec<bool, 3> &a) { return a.x && a.y && a.z; }

	template<>
	constexpr bool all(const vec<bool, 4> &a) { return a.x && a.y && a.z && a.w; }

	template<int n>
	vec<bool, n> bitvec(int bits);

	template<>
	constexpr vec2b bitvec(int bits) { return vec2b((bits & 1) != 0, (bits & 2) != 0); }

	template<>
	constexpr vec3b bitvec(int bits) { return vec3b((bits & 1) != 0, (bits & 2) != 0, (bits & 4) != 0); }

	template<>
	constexpr vec4b bitvec(int bits) { return vec4b((bits & 1) != 0, (bits & 2) != 0, (bits & 4) != 0, (bits & 8) != 0); }

	template<typename T, int n>
	vec<T, n> select(vec<bool, n> const &cond, vec<T, n> const &a, vec<T, n> const &b);

	template<typename T>
	constexpr vec<T, 2> select(const vec<bool, 2> &cond, const vec<T, 2> &a, const vec<T, 2> &b)
	{
		return vec<T, 2>(cond.x ? a.x : b.x, cond.y ? a.y : b.y);
	}

	template<typename T>
	constexpr vec<T, 3> select(const vec<bool, 3> &cond, const vec<T, 3> &a, const vec<T, 3> &b)
	{
		return vec<T, 3>(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z);
	}

	template<typename T>
	constexpr vec<T, 4> select(const vec<bool, 4> &cond, const vec<T, 4> &a, const vec<T, 4> &b)
	{
		return vec<T, 4>(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z, cond.w ? a.w : b.w);
	}

	template<typename T, int n>
	constexpr vec<T, n> min(vec<T, n> const &a, vec<T, n> const &b)
	{
		return select(a < b, a, b);
	}

	template<typename T, int n>
	constexpr vec<T, n> max(vec<T, n> const &a, vec<T, n> const &b)
	{
		return select(a < b, b, a);
	}

	template<typename T>
	constexpr vec<T, 2> min(vec<T, 2> const &a, vec<T, 2> const &b)
	{
		vec<T, 2> result;
		result.x = (a.x < b.x) ? a.x : b.x;
		result.y = (a.y < b.y) ? a.y : b.y;
		return result;
	}

	template<typename T>
	constexpr vec<T, 3> min(vec<T, 3> const &a, vec<T, 3> const &b)
	{
		vec<T, 3> result;
		result.x = (a.x < b.x) ? a.x : b.x;
		result.y = (a.y < b.y) ? a.y : b.y;
		result.z = (a.z < b.z) ? a.z : b.z;
		return result;
	}

	template<typename T>
	constexpr vec<T, 4> min(vec<T, 4> const &a, vec<T, 4> const &b)
	{
		vec<T, 4> result;
		result.x = (a.x < b.x) ? a.x : b.x;
		result.y = (a.y < b.y) ? a.y : b.y;
		result.z = (a.z < b.z) ? a.z : b.z;
		result.w = (a.w < b.w) ? a.w : b.w;
		return result;
	}

	template<typename T>
	constexpr vec<T, 2> max(vec<T, 2> const &a, vec<T, 2> const &b)
	{
		vec<T, 2> result;
		result.x = (a.x > b.x) ? a.x : b.x;
		result.y = (a.y > b.y) ? a.y : b.y;
		return result;
	}

	template<typename T>
	constexpr vec<T, 3> max(vec<T, 3> const &a, vec<T, 3> const &b)
	{
		vec<T, 3> result;
		result.x = (a.x > b.x) ? a.x : b.x;
		result.y = (a.y > b.y) ? a.y : b.y;
		result.z = (a.z > b.z) ? a.z : b.z;
		return result;
	}

	template<typename T>
	constexpr vec<T, 4> max(vec<T, 4> const &a, vec<T, 4> const &b)
	{
		vec<T, 4> result;
		result.x = (a.x > b.x) ? a.x : b.x;
		result.y = (a.y > b.y) ? a.y : b.y;
		result.z = (a.z > b.z) ? a.z : b.z;
		result.w = (a.w > b.w) ? a.w : b.w;
		return result;
	}

	template<typename T, int n>
	constexpr vec<T, n> abs(vec<T, n> const &a)
	{
		return select(a < T(0), -a, a);
	}

	template<typename T>
	constexpr vec<T, 2> abs(vec<T, 2> const &a)
	{
		vec<T, 2> result;
		result.x = std::abs(a.x);
		result.y = std::abs(a.y);
		return result;
	}

	template<typename T>
	constexpr vec<T, 3> abs(vec<T, 3> const &a)
	{
		vec<T, 3> result;
		result.x = std::abs(a.x);
		result.y = std::abs(a.y);
		result.z = std::abs(a.z);
		return result;
	}

	template<typename T>
	constexpr vec<T, 4> abs(vec<T, 4> const &a)
	{
		vec<T, 4> result;
		result.x = std::abs(a.x);
		result.y = std::abs(a.y);
		result.z = std::abs(a.z);
		result.w = std::abs(a.w);
		return result;
	}

	template<typename T, int n>
	vec<T, n> saturate(vec<T, n> const &value)
	{
		return clamp(value, vec<T, n>(T(0)), vec<T, n>(T(1)));
	}

	template<typename T, int n>
	T minComponent(vec<T, n> const &a)
	{
		T result = a[0];
		for (int i = 1; i < n; ++i)
			result = min(result, a[i]);
		return result;
	}

	template<typename T, int n>
	T maxComponent(vec<T, n> const &a)
	{
		T result = a[0];
		for (int i = 1; i < n; ++i)
			result = max(result, a[i]);
		return result;
	}

	template<int n>
	constexpr vec<float, n> degrees(vec<float, n> rad)
	{
		return rad * (180.f / constants::pi<float32>());
	}

	template<int n>
	constexpr vec<float, n> radians(vec<float, n> deg)
	{
		return deg * (constants::pi<float32>() / 180.f);
	}

	vec3f sphericalToCartesian(float azimuth, float elevation, float distance);
	vec3f sphericalDegreesToCartesian(float azimuth, float elevation, float distance);

	void cartesianToSpherical(const vec3f &v, float &azimuth, float &elevation, float &distance);
	void cartesianToSphericalDegrees(const vec3f &v, float &azimuth, float &elevation, float &distance);
}
