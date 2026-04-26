#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

namespace tsm
{
	// template<typename Type, unsigned int NDim>
	// struct Vector;
	//
	// template<typename Type>
	// struct Vector<Type, 2>
	// {
	// 	static constexpr unsigned int NDim = 2u;
	//
	// 	union
	// 	{
	// 		struct
	// 		{
	// 			Type x;
	// 			Type y;
	// 		};
	//
	// 		Type data[NDim];
	// 	};
	//
	// 	Vector() = default;
	//
	// 	Vector(Type p_x, Type p_y) : x(p_x), y(p_y)
	// 	{
	// 	}
	//
	// 	Vector(Type p_s) : x(p_s), y(p_s)
	// 	{
	// 	}
	// };

	// TODO:

	using bool1 = bool;                          //!< \brief boolean type with 1 component. (From GLM_GTX_compatibility extension)
	using bool2 = glm::vec<2, bool, glm::highp>; //!< \brief boolean type with 2 components. (From GLM_GTX_compatibility extension)
	using bool3 = glm::vec<3, bool, glm::highp>; //!< \brief boolean type with 3 components. (From GLM_GTX_compatibility extension)
	using bool4 = glm::vec<4, bool, glm::highp>; //!< \brief boolean type with 4 components. (From GLM_GTX_compatibility extension)

	using bool1x1 = bool;                             //!< \brief boolean matrix with 1 x 1 component. (From GLM_GTX_compatibility extension)
	using bool2x2 = glm::mat<2, 2, bool, glm::highp>; //!< \brief boolean matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	using bool2x3 = glm::mat<2, 3, bool, glm::highp>; //!< \brief boolean matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	using bool2x4 = glm::mat<2, 4, bool, glm::highp>; //!< \brief boolean matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	using bool3x2 = glm::mat<3, 2, bool, glm::highp>; //!< \brief boolean matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	using bool3x3 = glm::mat<3, 3, bool, glm::highp>; //!< \brief boolean matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	using bool3x4 = glm::mat<3, 4, bool, glm::highp>; //!< \brief boolean matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	using bool4x2 = glm::mat<4, 2, bool, glm::highp>; //!< \brief boolean matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	using bool4x3 = glm::mat<4, 3, bool, glm::highp>; //!< \brief boolean matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	using bool4x4 = glm::mat<4, 4, bool, glm::highp>; //!< \brief boolean matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	using int1 = int;                          //!< \brief integer vector with 1 component. (From GLM_GTX_compatibility extension)
	using int2 = glm::vec<2, int, glm::highp>; //!< \brief integer vector with 2 components. (From GLM_GTX_compatibility extension)
	using int3 = glm::vec<3, int, glm::highp>; //!< \brief integer vector with 3 components. (From GLM_GTX_compatibility extension)
	using int4 = glm::vec<4, int, glm::highp>; //!< \brief integer vector with 4 components. (From GLM_GTX_compatibility extension)

	using int1x1 = int;                             //!< \brief integer matrix with 1 component. (From GLM_GTX_compatibility extension)
	using int2x2 = glm::mat<2, 2, int, glm::highp>; //!< \brief integer matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	using int2x3 = glm::mat<2, 3, int, glm::highp>; //!< \brief integer matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	using int2x4 = glm::mat<2, 4, int, glm::highp>; //!< \brief integer matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	using int3x2 = glm::mat<3, 2, int, glm::highp>; //!< \brief integer matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	using int3x3 = glm::mat<3, 3, int, glm::highp>; //!< \brief integer matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	using int3x4 = glm::mat<3, 4, int, glm::highp>; //!< \brief integer matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	using int4x2 = glm::mat<4, 2, int, glm::highp>; //!< \brief integer matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	using int4x3 = glm::mat<4, 3, int, glm::highp>; //!< \brief integer matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	using int4x4 = glm::mat<4, 4, int, glm::highp>; //!< \brief integer matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	using float1 = float;                          //!< \brief single-qualifier floating-point vector with 1 component. (From GLM_GTX_compatibility extension)
	using float2 = glm::vec<2, float, glm::highp>; //!< \brief single-qualifier floating-point vector with 2 components. (From GLM_GTX_compatibility extension)
	using float3 = glm::vec<3, float, glm::highp>; //!< \brief single-qualifier floating-point vector with 3 components. (From GLM_GTX_compatibility extension)
	using float4 = glm::vec<4, float, glm::highp>; //!< \brief single-qualifier floating-point vector with 4 components. (From GLM_GTX_compatibility extension)

	using float1x1 = float;                             //!< \brief single-qualifier floating-point matrix with 1 component. (From GLM_GTX_compatibility extension)
	using float2x2 = glm::mat<2, 2, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	using float2x3 = glm::mat<2, 3, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	using float2x4 = glm::mat<2, 4, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	using float3x2 = glm::mat<3, 2, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	using float3x3 = glm::mat<3, 3, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	using float3x4 = glm::mat<3, 4, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	using float4x2 = glm::mat<4, 2, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	using float4x3 = glm::mat<4, 3, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	using float4x4 = glm::mat<4, 4, float, glm::highp>; //!< \brief single-qualifier floating-point matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	using double1 = double;                          //!< \brief double-qualifier floating-point vector with 1 component. (From GLM_GTX_compatibility extension)
	using double2 = glm::vec<2, double, glm::highp>; //!< \brief double-qualifier floating-point vector with 2 components. (From GLM_GTX_compatibility extension)
	using double3 = glm::vec<3, double, glm::highp>; //!< \brief double-qualifier floating-point vector with 3 components. (From GLM_GTX_compatibility extension)
	using double4 = glm::vec<4, double, glm::highp>; //!< \brief double-qualifier floating-point vector with 4 components. (From GLM_GTX_compatibility extension)

	using double1x1 = double;                             //!< \brief double-qualifier floating-point matrix with 1 component. (From GLM_GTX_compatibility extension)
	using double2x2 = glm::mat<2, 2, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	using double2x3 = glm::mat<2, 3, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	using double2x4 = glm::mat<2, 4, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	using double3x2 = glm::mat<3, 2, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	using double3x3 = glm::mat<3, 3, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	using double3x4 = glm::mat<3, 4, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	using double4x2 = glm::mat<4, 2, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	using double4x3 = glm::mat<4, 3, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	using double4x4 = glm::mat<4, 4, double, glm::highp>; //!< \brief double-qualifier floating-point matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	using quat = glm::quat;
}
