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

#include "vector.hpp"
#include "vector2.hpp"
#include "vector2i.hpp"

namespace tst
{
	template<typename Type>
	class _Rect
	{
	public:
		constexpr        _Rect()                  = default;
		constexpr        _Rect(const _Rect &)     = default;
		constexpr        _Rect(_Rect &&)          = default;
		constexpr _Rect &operator=(const _Rect &) = default;
		constexpr _Rect &operator=(_Rect &&)      = default;
		constexpr        ~_Rect()                 = default;

		union
		{
			struct
			{
				Type x;
				Type y;
				Type width;
				Type height;
			};

			Type data[4] = {};
		};

		constexpr _Rect(Type _x, Type _y, Type _width, Type _height) : x(_x), y(_y), width(_width), height(_height)
		{
		}

		constexpr _Rect(Type _s) : x(_s), y(_s), width(_s), height(_s)
		{
		}
	};

	using Rect   = _Rect<float32>;
	using RectI  = _Rect<int32>;
	using RectUI = _Rect<uint32>;

	// struct Rect2I
	// {
	// 	Rect2I() : x(0), y(0), width(0), height(0)
	// 	{
	// 	}
	//
	// 	Rect2I(int32 x, int32 y, int32 width, int32 height)
	// 		: x(x), y(y), width(width), height(height)
	// 	{
	// 	}
	//
	// 	Rect2I(Vector2I position, Vector2I resolution) : x(position.x), y(position.y), width(resolution.x), height(resolution.y)
	// 	{
	// 	}
	//
	// 	operator Rect2() const;
	//
	// 	union
	// 	{
	// 		struct
	// 		{
	// 			int32 x;
	// 			int32 y;
	// 			int32 width;
	// 			int32 height;
	// 		};
	//
	// 		struct
	// 		{
	// 			Vector2I position;
	// 			Vector2I resolution;
	// 		};
	// 	};
	//
	// 	bool operator==(const Rect2I &r) const;
	// 	bool operator!=(const Rect2I &r) const;
	//
	// 	bool intersects(const Rect2I &other) const
	// 	{
	// 		if (position.x >= (other.position.x + other.resolution.x))
	// 		{
	// 			return false;
	// 		}
	// 		if ((position.x + resolution.x) <= other.position.x)
	// 		{
	// 			return false;
	// 		}
	// 		if (position.y >= (other.position.y + other.resolution.y))
	// 		{
	// 			return false;
	// 		}
	// 		if ((position.y + resolution.y) <= other.position.y)
	// 		{
	// 			return false;
	// 		}
	// 		return true;
	// 	}
	//
	// 	Rect2I intersection(const Rect2I &rect) const
	// 	{
	// 		Rect2I new_rect = rect;
	//
	// 		if (!intersects(new_rect))
	// 		{
	// 			return Rect2I();
	// 		}
	//
	// 		new_rect.position = rect.position.max(position);
	//
	// 		Vector2 p_rect_end = rect.position + rect.resolution;
	// 		Vector2 end        = position + resolution;
	//
	// 		new_rect.resolution = p_rect_end.min(end) - new_rect.position;
	//
	// 		return new_rect;
	// 	}
	// };
	//
	// struct Rect2
	// {
	// 	Rect2() : x(0), y(0), width(0), height(0)
	// 	{
	// 	}
	//
	// 	Rect2(float32 x, float32 y, float32 width, float32 height)
	// 		: x(x), y(y), width(width), height(height)
	// 	{
	// 	}
	//
	// 	Rect2(Vector2 position, Vector2 resolution) : x(position.x), y(position.y), width(resolution.x), height(resolution.y)
	// 	{
	// 	}
	//
	// 	union
	// 	{
	// 		struct
	// 		{
	// 			float32 x;
	// 			float32 y;
	// 			float32 width;
	// 			float32 height;
	// 		};
	//
	// 		struct
	// 		{
	// 			Vector2 position;
	// 			Vector2 resolution;
	// 		};
	// 	};
	//
	// 	operator Rect2I() const;
	//
	// 	bool operator==(const Rect2 &r) const;
	// 	bool operator!=(const Rect2 &r) const;
	//
	// 	bool intersects(const Rect2 &other) const
	// 	{
	// 		if (position.x >= (other.position.x + other.resolution.x))
	// 		{
	// 			return false;
	// 		}
	// 		if ((position.x + resolution.x) <= other.position.x)
	// 		{
	// 			return false;
	// 		}
	// 		if (position.y >= (other.position.y + other.resolution.y))
	// 		{
	// 			return false;
	// 		}
	// 		if ((position.y + resolution.y) <= other.position.y)
	// 		{
	// 			return false;
	// 		}
	// 		return true;
	// 	}
	//
	// 	Rect2 intersection(const Rect2 &rect) const
	// 	{
	// 		Rect2 new_rect = rect;
	//
	// 		if (!intersects(new_rect))
	// 		{
	// 			return Rect2();
	// 		}
	//
	// 		new_rect.position = rect.position.max(position);
	//
	// 		Vector2 p_rect_end = rect.position + rect.resolution;
	// 		Vector2 end        = position + resolution;
	//
	// 		new_rect.resolution = p_rect_end.min(end) - new_rect.position;
	//
	// 		return new_rect;
	// 	}
	// };
	//
	// inline bool Rect2I::operator==(const Rect2I &r) const
	// {
	// 	return position == r.position && width == r.width && height == r.height;
	// }
	//
	// inline bool Rect2I::operator!=(const Rect2I &r) const
	// {
	// 	return position != r.position || width != r.width || height != r.height;
	// }
	//
	// inline Rect2I::operator Rect2() const
	// {
	// 	return {position, resolution};
	// }
	//
	// inline bool Rect2::operator==(const Rect2 &r) const
	// {
	// 	return position == r.position && width == r.width && height == r.height;
	// }
	//
	// inline bool Rect2::operator!=(const Rect2 &r) const
	// {
	// 	return position != r.position || width != r.width || height != r.height;
	// }
	//
	// inline Rect2::operator Rect2I() const
	// {
	// 	return {position, resolution};
	// }
}
