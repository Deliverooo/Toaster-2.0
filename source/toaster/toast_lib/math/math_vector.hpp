#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
// #include <glm/gtc/type_ptr.hpp>

namespace tsm
{
	// TODO:

	typedef bool                          bool1; //!< \brief boolean type with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::vec<2, bool, glm::highp> bool2; //!< \brief boolean type with 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<3, bool, glm::highp> bool3; //!< \brief boolean type with 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<4, bool, glm::highp> bool4; //!< \brief boolean type with 4 components. (From GLM_GTX_compatibility extension)

	typedef bool                             bool1x1; //!< \brief boolean matrix with 1 x 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 2, bool, glm::highp> bool2x2; //!< \brief boolean matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 3, bool, glm::highp> bool2x3; //!< \brief boolean matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 4, bool, glm::highp> bool2x4; //!< \brief boolean matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 2, bool, glm::highp> bool3x2; //!< \brief boolean matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 3, bool, glm::highp> bool3x3; //!< \brief boolean matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 4, bool, glm::highp> bool3x4; //!< \brief boolean matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 2, bool, glm::highp> bool4x2; //!< \brief boolean matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 3, bool, glm::highp> bool4x3; //!< \brief boolean matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 4, bool, glm::highp> bool4x4; //!< \brief boolean matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	typedef int                          int1; //!< \brief integer vector with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::vec<2, int, glm::highp> int2; //!< \brief integer vector with 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<3, int, glm::highp> int3; //!< \brief integer vector with 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<4, int, glm::highp> int4; //!< \brief integer vector with 4 components. (From GLM_GTX_compatibility extension)

	typedef int                             int1x1; //!< \brief integer matrix with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 2, int, glm::highp> int2x2; //!< \brief integer matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 3, int, glm::highp> int2x3; //!< \brief integer matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 4, int, glm::highp> int2x4; //!< \brief integer matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 2, int, glm::highp> int3x2; //!< \brief integer matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 3, int, glm::highp> int3x3; //!< \brief integer matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 4, int, glm::highp> int3x4; //!< \brief integer matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 2, int, glm::highp> int4x2; //!< \brief integer matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 3, int, glm::highp> int4x3; //!< \brief integer matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 4, int, glm::highp> int4x4; //!< \brief integer matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	typedef float                          float1; //!< \brief single-qualifier floating-point vector with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::vec<2, float, glm::highp> float2; //!< \brief single-qualifier floating-point vector with 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<3, float, glm::highp> float3; //!< \brief single-qualifier floating-point vector with 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<4, float, glm::highp> float4; //!< \brief single-qualifier floating-point vector with 4 components. (From GLM_GTX_compatibility extension)

	typedef float                             float1x1; //!< \brief single-qualifier floating-point matrix with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 2, float, glm::highp> float2x2; //!< \brief single-qualifier floating-point matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 3, float, glm::highp> float2x3; //!< \brief single-qualifier floating-point matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 4, float, glm::highp> float2x4; //!< \brief single-qualifier floating-point matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 2, float, glm::highp> float3x2; //!< \brief single-qualifier floating-point matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 3, float, glm::highp> float3x3; //!< \brief single-qualifier floating-point matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 4, float, glm::highp> float3x4; //!< \brief single-qualifier floating-point matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 2, float, glm::highp> float4x2; //!< \brief single-qualifier floating-point matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 3, float, glm::highp> float4x3; //!< \brief single-qualifier floating-point matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 4, float, glm::highp> float4x4; //!< \brief single-qualifier floating-point matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)

	typedef double                          double1; //!< \brief double-qualifier floating-point vector with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::vec<2, double, glm::highp> double2; //!< \brief double-qualifier floating-point vector with 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<3, double, glm::highp> double3; //!< \brief double-qualifier floating-point vector with 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::vec<4, double, glm::highp> double4; //!< \brief double-qualifier floating-point vector with 4 components. (From GLM_GTX_compatibility extension)

	typedef double                             double1x1; //!< \brief double-qualifier floating-point matrix with 1 component. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 2, double, glm::highp> double2x2; //!< \brief double-qualifier floating-point matrix with 2 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 3, double, glm::highp> double2x3; //!< \brief double-qualifier floating-point matrix with 2 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<2, 4, double, glm::highp> double2x4; //!< \brief double-qualifier floating-point matrix with 2 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 2, double, glm::highp> double3x2; //!< \brief double-qualifier floating-point matrix with 3 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 3, double, glm::highp> double3x3; //!< \brief double-qualifier floating-point matrix with 3 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<3, 4, double, glm::highp> double3x4; //!< \brief double-qualifier floating-point matrix with 3 x 4 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 2, double, glm::highp> double4x2; //!< \brief double-qualifier floating-point matrix with 4 x 2 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 3, double, glm::highp> double4x3; //!< \brief double-qualifier floating-point matrix with 4 x 3 components. (From GLM_GTX_compatibility extension)
	typedef glm::mat<4, 4, double, glm::highp> double4x4; //!< \brief double-qualifier floating-point matrix with 4 x 4 components. (From GLM_GTX_compatibility extension)
}
