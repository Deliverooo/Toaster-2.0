#pragma once

#include "core/core_api.hpp"

namespace tst
{
	struct Rect2I
	{
	public:
		Rect2I() = default;

		Rect2I(int x, int y, int width, int height)
			: x(x), y(y), width(width), height(height)
		{
		}

		int x      = 0;
		int y      = 0;
		int width  = 0;
		int height = 0;
	};
}
