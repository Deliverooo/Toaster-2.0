#pragma once
#include "toast_lib/system_types.h"

namespace tsm
{
	#pragma region vec2
	// Vec2
	template<typename Type>
	struct vec2
	{
		static constexpr uint32 s_dim{2u};

		union
		{
			Type data[s_dim];

			struct
			{
				Type x;
				Type y;
			};
		};

		constexpr vec2() : x(static_cast<Type>(0)), y(static_cast<Type>(0))
		{
		}

		constexpr vec2(Type p_x, Type p_y) : x(p_x), y(p_y)
		{
		};

		constexpr vec2(Type p_s) : x(p_s), y(p_s)
		{
		}

		constexpr Type &operator[](int32 p_index)
		{
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const Type &
		{
			return data[p_index];
		}

		// Unary
		constexpr auto operator+(const vec2 &p_v) -> vec2
		{
			return p_v;
		}

		constexpr auto operator-(const vec2 &p_v) -> vec2
		{
			return vec2{-p_v.x, -p_v.y};
		}
	};

	// Binary
	template<typename Type>
	constexpr auto operator+(const vec2<Type> &p_v, Type p_s) -> vec2<Type> { return vec2{p_v.x + p_s, p_v.y + p_s}; }

	template<typename Type>
	constexpr auto operator+(Type p_s, const vec2<Type> &p_v) -> vec2<Type> { return vec2{p_s + p_v.x, p_s + p_v.y}; }

	template<typename Type>
	constexpr auto operator+(const vec2<Type> &p_v1, const vec2<Type> &p_v2) -> vec2<Type> { return vec2{p_v1.x + p_v2.x, p_v1.y + p_v2.y}; }

	template<typename Type>
	constexpr auto operator-(const vec2<Type> &p_v, Type p_s) -> vec2<Type> { return vec2{p_v.x - p_s, p_v.y - p_s}; }

	template<typename Type>
	constexpr auto operator-(Type p_s, const vec2<Type> &p_v) -> vec2<Type> { return vec2{p_s - p_v.x, p_s - p_v.y}; }

	template<typename Type>
	constexpr auto operator-(const vec2<Type> &p_v1, const vec2<Type> &p_v2) -> vec2<Type> { return vec2{p_v1.x - p_v2.x, p_v1.y - p_v2.y}; }

	template<typename Type>
	constexpr auto operator*(const vec2<Type> &p_v, Type p_s) -> vec2<Type> { return vec2{p_v.x * p_s, p_v.y * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const vec2<Type> &p_v) -> vec2<Type> { return vec2{p_s * p_v.x, p_s * p_v.y}; }

	template<typename Type>
	constexpr auto operator/(const vec2<Type> &p_v, Type p_s) -> vec2<Type> { return vec2{p_v.x / p_s, p_v.y / p_s}; }

	template<typename Type>
	constexpr auto operator/(Type p_s, const vec2<Type> &p_v) -> vec2<Type> { return vec2{p_s / p_v.x, p_s / p_v.y}; }

	#pragma endregion

	#pragma region vec3
	// Vec3
	template<typename Type>
	struct vec3
	{
		static constexpr uint32 s_dim{3u};

		union
		{
			Type data[s_dim];

			struct
			{
				Type x;
				Type y;
				Type z;
			};
		};

		constexpr vec3() : x(static_cast<Type>(0)), y(static_cast<Type>(0)), z(static_cast<Type>(0))
		{
		}

		constexpr vec3(Type p_x, Type p_y, Type p_z) : x(p_x), y(p_y), z(p_z)
		{
		};

		constexpr vec3(Type p_s) : x(p_s), y(p_s), z(p_s)
		{
		}

		constexpr Type &operator[](int32 p_index)
		{
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const Type &
		{
			return data[p_index];
		}

		// Unary
		constexpr auto operator+(const vec3 &p_v) -> vec3
		{
			return p_v;
		}

		constexpr auto operator-(const vec3 &p_v) -> vec3
		{
			return vec2{-p_v.x, -p_v.y, -p_v.z};
		}
	};

	// Binary
	template<typename Type>
	constexpr auto operator+(const vec3<Type> &p_v, Type p_s) -> vec3<Type> { return vec3{p_v.x + p_s, p_v.y + p_s, p_v.z + p_s}; }

	template<typename Type>
	constexpr auto operator+(Type p_s, const vec3<Type> &p_v) -> vec3<Type> { return vec3{p_s + p_v.x, p_s + p_v.y, p_s + p_v.z}; }

	template<typename Type>
	constexpr auto operator+(const vec3<Type> &p_v1, const vec3<Type> &p_v2) -> vec3<Type> { return vec3{p_v1.x + p_v2.x, p_v1.y + p_v2.y, p_v1.z + p_v2.z}; }

	template<typename Type>
	constexpr auto operator-(const vec3<Type> &p_v, Type p_s) -> vec3<Type> { return vec3{p_v.x - p_s, p_v.y - p_s, p_v.z - p_s}; }

	template<typename Type>
	constexpr auto operator-(Type p_s, const vec3<Type> &p_v) -> vec3<Type> { return vec3{p_s - p_v.x, p_s - p_v.y, p_s - p_v.z}; }

	template<typename Type>
	constexpr auto operator-(const vec3<Type> &p_v1, const vec3<Type> &p_v2) -> vec3<Type> { return vec3{p_v1.x - p_v2.x, p_v1.y - p_v2.y, p_v1.z - p_v2.z}; }

	template<typename Type>
	constexpr auto operator*(const vec3<Type> &p_v, Type p_s) -> vec3<Type> { return vec3{p_v.x * p_s, p_v.y * p_s, p_v.z * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const vec3<Type> &p_v) -> vec3<Type> { return vec3{p_s * p_v.x, p_s * p_v.y, p_s * p_v.z}; }

	template<typename Type>
	constexpr auto operator/(const vec3<Type> &p_v, Type p_s) -> vec3<Type> { return vec3{p_v.x / p_s, p_v.y / p_s, p_v.z / p_s}; }

	template<typename Type>
	constexpr auto operator/(Type p_s, const vec3<Type> &p_v) -> vec3<Type> { return vec3{p_s / p_v.x, p_s / p_v.y, p_s / p_v.z}; }
	#pragma endregion

	#pragma region vec4
	// Vec4
	template<typename Type>
	struct vec4
	{
		static constexpr uint32 s_dim{4u};

		union
		{
			Type data[s_dim];

			struct
			{
				Type x;
				Type y;
				Type z;
				Type w;
			};
		};

		constexpr vec4() : x(static_cast<Type>(0)), y(static_cast<Type>(0)), z(static_cast<Type>(0)), w(static_cast<Type>(0))
		{
		}

		constexpr vec4(Type p_x, Type p_y, Type p_z, Type p_w) : x(p_x), y(p_y), z(p_z), w(p_w)
		{
		};

		constexpr vec4(Type p_s) : x(p_s), y(p_s), z(p_s), w(p_s)
		{
		}

		constexpr Type &operator[](int32 p_index)
		{
			return data[p_index];
		}

		constexpr auto operator[](int32 p_index) const -> const Type &
		{
			return data[p_index];
		}

		// Unary
		constexpr auto operator+(const vec4 &p_v) -> vec4
		{
			return p_v;
		}

		constexpr auto operator-(const vec4 &p_v) -> vec4
		{
			return vec2{-p_v.x, -p_v.y, -p_v.z, -p_v.w};
		}
	};

	// Binary
	template<typename Type>
	constexpr auto operator+(const vec4<Type> &p_v, Type p_s) -> vec4<Type> { return vec4{p_v.x + p_s, p_v.y + p_s, p_v.z + p_s, p_v.w + p_s}; }

	template<typename Type>
	constexpr auto operator+(Type p_s, const vec4<Type> &p_v) -> vec4<Type> { return vec4{p_s + p_v.x, p_s + p_v.y, p_s + p_v.z, p_s + p_v.w}; }

	template<typename Type>
	constexpr auto operator+(const vec4<Type> &p_v1, const vec4<Type> &p_v2) -> vec4<Type>
	{
		return vec4{p_v1.x + p_v2.x, p_v1.y + p_v2.y, p_v1.z + p_v2.z, p_v1.w + p_v2.w};
	}

	template<typename Type>
	constexpr auto operator-(const vec4<Type> &p_v, Type p_s) -> vec4<Type> { return vec4{p_v.x - p_s, p_v.y - p_s, p_v.z - p_s, p_v.w - p_s}; }

	template<typename Type>
	constexpr auto operator-(Type p_s, const vec4<Type> &p_v) -> vec4<Type> { return vec4{p_s - p_v.x, p_s - p_v.y, p_s - p_v.z, p_s - p_v.w}; }

	template<typename Type>
	constexpr auto operator-(const vec4<Type> &p_v1, const vec4<Type> &p_v2) -> vec4<Type>
	{
		return vec4{p_v1.x - p_v2.x, p_v1.y - p_v2.y, p_v1.z - p_v2.z, p_v1.w - p_v2.w};
	}

	template<typename Type>
	constexpr auto operator*(const vec4<Type> &p_v, Type p_s) -> vec4<Type> { return vec4{p_v.x * p_s, p_v.y * p_s, p_v.z * p_s, p_v.w * p_s}; }

	template<typename Type>
	constexpr auto operator*(Type p_s, const vec4<Type> &p_v) -> vec4<Type> { return vec4{p_s * p_v.x, p_s * p_v.y, p_s * p_v.z, p_s * p_v.w}; }

	template<typename Type>
	constexpr auto operator/(const vec4<Type> &p_v, Type p_s) -> vec4<Type> { return vec4{p_v.x / p_s, p_v.y / p_s, p_v.z / p_s, p_v.w / p_s}; }

	template<typename Type>
	constexpr auto operator/(Type p_s, const vec4<Type> &p_v) -> vec4<Type> { return vec4{p_s / p_v.x, p_s / p_v.y, p_s / p_v.z, p_s / p_v.w}; }
	#pragma endregion

	using bool1 = bool32;
	using bool2 = vec2<bool32>;
	using bool3 = vec3<bool32>;
	using bool4 = vec4<bool32>;

	using int1 = int32;
	using int2 = vec2<int32>;
	using int3 = vec3<int32>;
	using int4 = vec4<int32>;

	using uint1 = uint32;
	using uint2 = vec2<uint32>;
	using uint3 = vec3<uint32>;
	using uint4 = vec4<uint32>;

	using float1 = float32;
	using float2 = vec2<float32>;
	using float3 = vec3<float32>;
	using float4 = vec4<float32>;

	using double1 = float64;
	using double2 = vec2<float64>;
	using double3 = vec3<float64>;
	using double4 = vec4<float64>;
}
