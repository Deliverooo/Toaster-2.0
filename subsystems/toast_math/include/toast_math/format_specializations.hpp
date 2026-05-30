#pragma once

#ifdef TST_MATH_FORMAT_SPECIALIZATIONS

#include "quaternion.hpp"

#include <fmt/format.h>

template<typename Type>
struct fmt::formatter<tsm::Vec2<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Vec2<Type> &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {})", v.x, v.y);
	}
};

template<typename Type>
struct fmt::formatter<tsm::Vec3<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Vec3<Type> &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
	}
};

template<typename Type>
struct fmt::formatter<tsm::Vec4<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Vec4<Type> &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
	}
};

template<typename Type>
struct fmt::formatter<tsm::Quat<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Quat<Type> &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
	}
};

template<typename Type>
struct fmt::formatter<tsm::Mat2<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Mat2<Type> &p_m, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "[{}, {}\n{}, {}]", p_m[0].x, p_m[1].x, p_m[0].y, p_m[1].y);
	}
};

template<typename Type>
struct fmt::formatter<tsm::Mat3<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Mat3<Type> &p_m, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "[{}, {}, {}\n{}, {}, {}\n{}, {}, {}]", p_m[0].x, p_m[1].x, p_m[2].x, p_m[0].y, p_m[1].y, p_m[2].y, p_m[0].z, p_m[1].z, p_m[2].z);
	}
};

template<typename Type>
struct fmt::formatter<tsm::Mat4<Type> >
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const tsm::Mat4<Type> &p_m, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "[{}, {}, {}, {}\n{}, {}, {}, {}\n{}, {}, {}, {}\n{}, {}, {}, {}]", p_m[0].x, p_m[1].x, p_m[2].x, p_m[3].x, p_m[0].y, p_m[1].y,
						 p_m[2].y, p_m[3].y, p_m[0].z, p_m[1].z, p_m[2].z, p_m[3].z, p_m[0].w, p_m[1].w, p_m[2].w, p_m[3].w);
	}
};
#endif
