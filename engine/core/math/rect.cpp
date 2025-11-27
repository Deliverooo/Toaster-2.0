#include "rect.hpp"

namespace tst
{
	bool Rect2I::operator==(const Rect2I &r) const
	{
		return position == r.position && width == r.width && height == r.height;
	}

	bool Rect2I::operator!=(const Rect2I &r) const
	{
		return position != r.position || width != r.width || height != r.height;
	}
}
