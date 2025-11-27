#pragma once

#include "vector2i.hpp"

namespace tst
{
	struct Rect2I
	{
		Rect2I() : x(0), y(0), width(0), height(0)
		{
		}

		Rect2I(int x, int y, int width, int height)
			: x(x), y(y), width(width), height(height)
		{
		}

		Rect2I(Vector2I position, Vector2I resolution) : x(position.x), y(position.y), width(resolution.x), height(resolution.y)
		{
		}

		union
		{
			struct
			{
				int x      = 0;
				int y      = 0;
				int width  = 0;
				int height = 0;
			};

			struct
			{
				Vector2I position;
				Vector2I resolution;
			};
		};

		bool operator==(const Rect2I &r) const;
		bool operator!=(const Rect2I &r) const;
	};
}
