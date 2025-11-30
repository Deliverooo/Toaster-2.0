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
#include "core_typedefs.hpp"
#include "error_macros.hpp"

namespace tst
{
	template<typename Type, uint32 Ndim>
	class Vector;

	template<typename Type>
	class Vector<Type, 2>
	{
	public:
		static constexpr uint32 Ndim = 2;

		constexpr         Vector()                  = default;
		constexpr         Vector(const Vector &)    = default;
		constexpr         Vector(Vector &&)         = default;
		constexpr Vector &operator=(const Vector &) = default;
		constexpr Vector &operator=(Vector &&)      = default;
		constexpr         ~Vector()                 = default;

		union
		{
			struct
			{
				Type x;
				Type y;
			};

			Type data[Ndim] = {};
		};

		constexpr Vector(Type _x, Type _y) : x(_x), y(_y)
		{
		}

		constexpr Vector(Type _s) : x(_s), y(_s)
		{
		}

		Type &operator[](int32 _index)
		{
			TST_ASSERT(_index < Ndim);
			return data[_index];
		}

		const Type &operator[](int32 _index) const
		{
			TST_ASSERT(_index < Ndim);
			return data[_index];
		}
	};

	template<typename Type>
	class Vector<Type, 3>
	{
	public:
		static constexpr uint32 Ndim = 3;

		constexpr         Vector()                  = default;
		constexpr         Vector(const Vector &)    = default;
		constexpr         Vector(Vector &&)         = default;
		constexpr Vector &operator=(const Vector &) = default;
		constexpr Vector &operator=(Vector &&)      = default;
		constexpr         ~Vector()                 = default;

		union
		{
			struct
			{
				Type x;
				Type y;
				Type z;
			};

			Type data[Ndim] = {};
		};

		Vector(Type _x, Type _y, Type _z) : x(_x), y(_y), z(_z)
		{
		}

		constexpr Vector(Type _s) : x(_s), y(_s), z(_s)
		{
		}

		Type &operator[](int32 _index)
		{
			TST_ASSERT(_index < Ndim);
			return data[_index];
		}

		const Type &operator[](int32 _index) const
		{
			TST_ASSERT(_index < Ndim);
			return data[_index];
		}
	};

	template<typename Type>
	class Vector<Type, 4>
	{
	public:
		static constexpr uint32 Ndim = 4;

		constexpr         Vector()                  = default;
		constexpr         Vector(const Vector &)    = default;
		constexpr         Vector(Vector &&)         = default;
		constexpr Vector &operator=(const Vector &) = default;
		constexpr Vector &operator=(Vector &&)      = default;
		constexpr         ~Vector()                 = default;

		union
		{
			struct
			{
				Type x;
				Type y;
				Type z;
				Type w;
			};

			Type data[Ndim] = {};
		};

		constexpr Vector(Type _x, Type _y, Type _z, Type _w) : x(_x), y(_y), z(_z), w(_w)
		{
		}

		constexpr Vector(Type _s) : x(_s), y(_s), z(_s), w(_s)
		{
		}

		Type &operator[](int32 _index)
		{
			TST_ASSERT(_index < Ndim);
			return data[_index];
		}

		const Type &operator[](int32 _index) const
		{
			TST_ASSERT(_index < Ndim);
			return data[_index];
		}
	};

	using Vec2 = Vector<float32, 2>;
	using Vec3 = Vector<float32, 3>;
	using Vec4 = Vector<float32, 4>;

	using Vec2_64 = Vector<float64, 2>;
	using Vec3_64 = Vector<float64, 3>;
	using Vec4_64 = Vector<float64, 4>;

	using Vec2i = Vector<int32, 2>;
	using Vec3i = Vector<int32, 3>;
	using Vec4i = Vector<int32, 4>;

	using Vec2ui = Vector<uint32, 2>;
	using Vec3ui = Vector<uint32, 3>;
	using Vec4ui = Vector<uint32, 4>;

	using Vec2i64 = Vector<int64, 2>;
	using Vec3i64 = Vector<int64, 3>;
	using Vec4i64 = Vector<int64, 4>;

	using Vec2ui64 = Vector<uint64, 2>;
	using Vec3ui64 = Vector<uint64, 3>;
	using Vec4ui64 = Vector<uint64, 4>;
}
