#pragma once


#include "quaternion.hpp"

#include <format>

template<>
struct std::formatter<Dx::XMFLOAT2>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const Dx::XMFLOAT2 &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {})", v.x, v.y);
	}
};

template<>
struct std::formatter<Dx::XMFLOAT3>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const Dx::XMFLOAT3 &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {})", v.x, v.y, v.z);
	}
};

template<>
struct std::formatter<Dx::XMFLOAT4>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const Dx::XMFLOAT4 &v, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
	}
};

template<>
struct std::formatter<Dx::XMFLOAT4X4>
{
	constexpr auto parse(format_parse_context &p_ctx)
	{
		return p_ctx.begin();
	}

	auto format(const Dx::XMFLOAT4X4 &m, format_context &p_ctx) const
	{
		return format_to(p_ctx.out(), "[{:.2f}, {:.2f}, {:.2f}, {:.2f}\n{:.2f}, {:.2f}, {:.2f}, {:.2f}\n{:.2f}, {:.2f}, {:.2f}, {:.2f}\n{:.2f}, {:.2f}, {:.2f}, {:.2f}]",
						 m._11, m._12, m._13, m._14, m._21, m._22, m._23, m._24, m._31, m._32, m._33, m._34, m._41, m._42, m._43, m._44);
	}
};

template<typename Type>
struct std::formatter<tsm::Vec2<Type> >
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
struct std::formatter<tsm::Vec3<Type> >
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
struct std::formatter<tsm::Vec4<Type> >
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
struct std::formatter<tsm::Quat<Type> >
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
struct std::formatter<tsm::Mat2<Type> >
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
struct std::formatter<tsm::Mat3<Type> >
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
struct std::formatter<tsm::Mat4<Type> >
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
