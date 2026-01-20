#pragma once

#include <compare>
#include <type_traits>

#include "gl_types.hpp"
#include "glad/glad.h"

namespace gl
{
	template<typename FlagBitsType>
	struct FlagTraits
	{
		static constexpr bool isBitmask = false;
	};

	template<typename BitType>
	class Flags
	{
	public:
		using BitsType = BitType;
		using MaskType = typename std::underlying_type_t<BitType>;

		// constructors
		constexpr Flags() noexcept : m_mask(0)
		{
		}

		constexpr Flags(BitType bit) noexcept : m_mask(static_cast<MaskType>(bit))
		{
		}

		constexpr Flags(Flags<BitType> const &rhs) noexcept = default;

		constexpr explicit Flags(MaskType flags) noexcept : m_mask(flags)
		{
		}

		// relational operators
		auto operator<=>(Flags<BitType> const &) const = default;

		constexpr bool operator<(Flags<BitType> const &rhs) const noexcept
		{
			return m_mask < rhs.m_mask;
		}

		constexpr bool operator<=(Flags<BitType> const &rhs) const noexcept
		{
			return m_mask <= rhs.m_mask;
		}

		constexpr bool operator>(Flags<BitType> const &rhs) const noexcept
		{
			return m_mask > rhs.m_mask;
		}

		constexpr bool operator>=(Flags<BitType> const &rhs) const noexcept
		{
			return m_mask >= rhs.m_mask;
		}

		constexpr bool operator==(Flags<BitType> const &rhs) const noexcept
		{
			return m_mask == rhs.m_mask;
		}

		constexpr bool operator!=(Flags<BitType> const &rhs) const noexcept
		{
			return m_mask != rhs.m_mask;
		}

		// logical operator
		constexpr bool operator!() const noexcept
		{
			return !m_mask;
		}

		// bitwise operators
		constexpr Flags<BitType> operator&(Flags<BitType> const &rhs) const noexcept
		{
			return Flags<BitType>(m_mask & rhs.m_mask);
		}

		constexpr Flags<BitType> operator|(Flags<BitType> const &rhs) const noexcept
		{
			return Flags<BitType>(m_mask | rhs.m_mask);
		}

		constexpr Flags<BitType> operator^(Flags<BitType> const &rhs) const noexcept
		{
			return Flags<BitType>(m_mask ^ rhs.m_mask);
		}

		constexpr Flags<BitType> operator~() const noexcept
		{
			return Flags<BitType>(m_mask ^ FlagTraits<BitType>::allFlags.m_mask);
		}

		// assignment operators
		constexpr Flags<BitType> &operator=(Flags<BitType> const &rhs) noexcept = default;

		constexpr Flags<BitType> &operator|=(Flags<BitType> const &rhs) noexcept
		{
			m_mask |= rhs.m_mask;
			return *this;
		}

		constexpr Flags<BitType> &operator&=(Flags<BitType> const &rhs) noexcept
		{
			m_mask &= rhs.m_mask;
			return *this;
		}

		constexpr Flags<BitType> &operator^=(Flags<BitType> const &rhs) noexcept
		{
			m_mask ^= rhs.m_mask;
			return *this;
		}

		// cast operators
		explicit constexpr operator bool() const noexcept
		{
			return !!m_mask;
		}

		constexpr operator MaskType() const noexcept
		{
			return m_mask;
		}

	private:
		MaskType m_mask;
	};

	// bitwise operators
	template<typename BitType>
	constexpr Flags<BitType> operator&(BitType bit, Flags<BitType> const &flags) noexcept
	{
		return flags.operator&(bit);
	}

	template<typename BitType>
	constexpr Flags<BitType> operator|(BitType bit, Flags<BitType> const &flags) noexcept
	{
		return flags.operator|(bit);
	}

	template<typename BitType>
	constexpr Flags<BitType> operator^(BitType bit, Flags<BitType> const &flags) noexcept
	{
		return flags.operator^(bit);
	}

	// bitwise operators on BitType
	template<typename BitType, std::enable_if_t<FlagTraits<BitType>::isBitmask, bool> = true>
	inline constexpr Flags<BitType> operator&(BitType lhs, BitType rhs) noexcept
	{
		return Flags<BitType>(lhs) & rhs;
	}

	template<typename BitType, std::enable_if_t<FlagTraits<BitType>::isBitmask, bool> = true>
	inline constexpr Flags<BitType> operator|(BitType lhs, BitType rhs) noexcept
	{
		return Flags<BitType>(lhs) | rhs;
	}

	template<typename BitType, std::enable_if_t<FlagTraits<BitType>::isBitmask, bool> = true>
	inline constexpr Flags<BitType> operator^(BitType lhs, BitType rhs) noexcept
	{
		return Flags<BitType>(lhs) ^ rhs;
	}

	template<typename BitType, std::enable_if_t<FlagTraits<BitType>::isBitmask, bool> = true>
	inline constexpr Flags<BitType> operator~(BitType bit) noexcept
	{
		return ~(Flags<BitType>(bit));
	}

	// Source -> Windows SDK's <gl/GL.h>
	enum class ClearMaskBits : Bitfield
	{
		eColor = 0x00004000, eDepth = 0x00000100, eStencil = 0x00000400
	};

	using ClearMaskFlags = Flags<ClearMaskBits>;

	template<>
	struct FlagTraits<ClearMaskBits>
	{
		static constexpr bool           isBitmask = true;
		static constexpr ClearMaskFlags allFlags  = ClearMaskBits::eColor | ClearMaskBits::eDepth | ClearMaskBits::eStencil;
	};

	/** If the parameters of a function call do not match the set of parameters allowed by OpenGL,
	 * or do not interact reasonably with state that is already set in the context, then an OpenGL Error will result.
	 * The errors are presented as an error code.
	 * For most OpenGL errors, and for most OpenGL functions, a function that emits an error will have no effect.
	 * No OpenGL state will be changed, no rendering will be initiated. It will be as if the function had not been called.
	 * There are a few cases where this is not the case.
	 *
	 * Source -> https://wikis.khronos.org/opengl/OpenGL_Error **/
	enum class Error : Enum
	{
		eNoError = 0x00000000,
		/** Given when an enumeration parameter is not a legal enumeration for that function.
		 * This is given only for local problems; if the spec allows the enumeration in certain circumstances,
		 * where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead. **/
		eInvalidEnum = 0x0500,
		/** Given when a value parameter is not a legal value for that function. This is only given for local problems;
		 * if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances,
		 * then GL_INVALID_OPERATION is the result instead. **/
		eInvalidValue = 0x0501,
		/** Given when the set of state for a command is not legal for the parameters given to that command.
		 * It is also given for commands where combinations of parameters define what the legal parameters are. **/
		eInvalidOperation = 0x0502,
		/** Given when a stack pushing operation cannot be done because it would overflow the limit of that stack's size. **/
		eStackOverflow = 0x0503,
		/** Given when a stack popping operation cannot be done because the stack is already at its lowest point. **/
		eStackUnderflow = 0x0504,
		/** Given when performing an operation that can allocate memory, and the memory cannot be allocated.
		 * The results of OpenGL functions that return this error are undefined;
		 * it is allowable for partial execution of an operation to happen in this circumstance. **/
		eOutOfMemory = 0x0505,
		/** Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete. **/
		eInvalidFramebufferOperation = 0x0506,
		/** Given if the OpenGL context has been lost, due to a graphics card reset. **/
		eContextLost = 0x0507,
		/** Part of the ARB_imaging extension.
		 * this error code is deprecated in 3.0 and removed in 3.1 core and above. **/
		eTableTooLarge = 0x8031
	};

	enum class DebugSource : Enum
	{
		/**	Calls to the OpenGL API **/
		eAPI = GL_DEBUG_SOURCE_API,

		/**	Calls to a window-system API **/
		eWindowSystem = GL_DEBUG_SOURCE_WINDOW_SYSTEM,

		/**	A compiler for a shading language **/
		eShaderCompiler = GL_DEBUG_SOURCE_SHADER_COMPILER,

		/**	An application associated with OpenGL **/
		eThirdParty = GL_DEBUG_SOURCE_THIRD_PARTY,

		/**	Generated by the user of this application **/
		eApplication = GL_DEBUG_SOURCE_APPLICATION,

		/**	Some source that isn't one of these **/
		eOther = GL_DEBUG_SOURCE_OTHER,
	};

	enum class DebugType : Enum
	{
		/**	An error, typically from the API **/
		eError = GL_DEBUG_TYPE_ERROR,

		/**	Some behavior marked deprecated has been used **/
		eDeprecatedBehavior = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,

		/**	Something has invoked undefined behavior **/
		eUndefinedBehavior = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,

		/**	Some functionality the user relies upon is not portable **/
		ePortability = GL_DEBUG_TYPE_PORTABILITY,

		/**	Code has triggered possible performance issues **/
		ePerformance = GL_DEBUG_TYPE_PERFORMANCE,

		/**	Command stream annotation **/
		eMarker = GL_DEBUG_TYPE_MARKER,

		/**	Group pushing **/
		ePushGroup = GL_DEBUG_TYPE_PUSH_GROUP,

		/**	Group popping **/
		ePopGroup = GL_DEBUG_TYPE_POP_GROUP,

		/**	Some type that isn't one of these **/
		eOther = GL_DEBUG_TYPE_OTHER,
	};

	enum class DebugSeverity : Enum
	{
		/**	All OpenGL Errors, shader compilation/linking errors, or highly-dangerous undefined behavior **/
		eHigh = GL_DEBUG_SEVERITY_HIGH,

		/**	Major performance warnings, shader compilation/linking warnings, or the use of deprecated functionality **/
		eMedium = GL_DEBUG_SEVERITY_MEDIUM,

		/**	Redundant state change performance warning, or unimportant undefined behavior **/
		eLow = GL_DEBUG_SEVERITY_LOW,

		/**	Anything that isn't an error or performance issue. **/
		eNotification = GL_DEBUG_SEVERITY_NOTIFICATION
	};

	enum class ShaderStage : Enum
	{
		eVertex         = GL_VERTEX_SHADER,
		eFragment       = GL_FRAGMENT_SHADER,
		eGeometry       = GL_GEOMETRY_SHADER,
		eCompute        = GL_COMPUTE_SHADER,
		eTessControl    = GL_TESS_CONTROL_SHADER,
		eTessEvaluation = GL_TESS_EVALUATION_SHADER
	};

	enum class PipelineShaderStageFlagBits : Bitfield
	{
		eVertex         = GL_VERTEX_SHADER_BIT,
		eFragment       = GL_FRAGMENT_SHADER_BIT,
		eGeometry       = GL_GEOMETRY_SHADER_BIT,
		eCompute        = GL_COMPUTE_SHADER_BIT,
		eTessControl    = GL_TESS_CONTROL_SHADER_BIT,
		eTessEvaluation = GL_TESS_EVALUATION_SHADER_BIT
	};

	using PipelineShaderStageFlags = Flags<PipelineShaderStageFlagBits>;

	template<>
	struct FlagTraits<PipelineShaderStageFlagBits>
	{
		static constexpr bool                     isBitmask = true;
		static constexpr PipelineShaderStageFlags allFlags  = PipelineShaderStageFlagBits::eVertex | PipelineShaderStageFlagBits::eFragment |
															  PipelineShaderStageFlagBits::eGeometry | PipelineShaderStageFlagBits::eCompute |
															  PipelineShaderStageFlagBits::eTessControl | PipelineShaderStageFlagBits::eTessEvaluation;
	};

	enum class BufferType : Enum
	{
		eArray             = GL_ARRAY_BUFFER,
		eAtomicCounter     = GL_ATOMIC_COUNTER_BUFFER,
		eCopyRead          = GL_COPY_READ_BUFFER,
		eCopyWrite         = GL_COPY_WRITE_BUFFER,
		eDrawIndirect      = GL_DRAW_INDIRECT_BUFFER,
		eDispatchIndirect  = GL_DISPATCH_INDIRECT_BUFFER,
		eElementArray      = GL_ELEMENT_ARRAY_BUFFER,
		ePixelPack         = GL_PIXEL_PACK_BUFFER,
		ePixelUnpack       = GL_PIXEL_UNPACK_BUFFER,
		eQuery             = GL_QUERY_BUFFER,
		eShaderStorage     = GL_SHADER_STORAGE_BUFFER,
		eTexture           = GL_TEXTURE_BUFFER,
		eTransformFeedback = GL_TRANSFORM_FEEDBACK_BUFFER,
		eUniform           = GL_UNIFORM_BUFFER
	};

	enum class BufferUsage : Enum
	{
		/** The data store contents will be modified once and used at most a few times. **/
		eStreamDraw = GL_STREAM_DRAW,

		/** The data store contents will be modified once and used many times. **/
		eStaticDraw = GL_STATIC_DRAW,

		/**The data store contents will be modified repeatedly and used many times. **/
		eDynamicDraw = GL_DYNAMIC_DRAW
	};

	enum class DataType : Enum
	{
		eHalfFloat                   = GL_HALF_FLOAT,
		eFloat                       = GL_FLOAT,
		eDouble                      = GL_DOUBLE,
		eFixed                       = GL_FIXED,
		eByte                        = GL_BYTE,
		eUnsignedByte                = GL_UNSIGNED_BYTE,
		eShort                       = GL_SHORT,
		eUnsignedShort               = GL_UNSIGNED_SHORT,
		eInt                         = GL_INT,
		eBool                        = GL_BOOL,
		eUnsignedInt                 = GL_UNSIGNED_INT,
		eInt_2_10_10_10_Rev          = GL_INT_2_10_10_10_REV,
		eUnsignedInt_2_10_10_10_Rev  = GL_UNSIGNED_INT_2_10_10_10_REV,
		eUnsignedInt_10F_11F_11F_Rev = GL_UNSIGNED_INT_10F_11F_11F_REV
	};

	enum class DrawMode : Enum
	{
		ePoints                 = GL_POINTS,
		eLineStrip              = GL_LINE_STRIP,
		eLineLoop               = GL_LINE_LOOP,
		eLines                  = GL_LINES,
		eLineStripAdjacency     = GL_LINE_STRIP_ADJACENCY,
		eLinesAdjacency         = GL_LINES_ADJACENCY,
		eTriangleStrip          = GL_TRIANGLE_STRIP,
		eTriangleFan            = GL_TRIANGLE_FAN,
		eTriangles              = GL_TRIANGLES,
		eTriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
		eTrianglesAdjacency     = GL_TRIANGLES_ADJACENCY,
		ePatches                = GL_PATCHES
	};

	/** See -> https://wikis.khronos.org/opengl/GLAPI/glEnable **/
	enum class Capability : Enum
	{
		/** If enabled, blend the computed fragment color values with the values in the color buffers.
		 * See glBlendFunc. Sets the blend enable/disable flag for all color buffers. **/
		eBlend = GL_BLEND,

		/** If enabled, clip geometry against user-defined half space i. **/
		eClipDistance0 = GL_CLIP_DISTANCE0,

		/** If enabled, apply the currently selected logical operation to the computed fragment color and color buffer values. See glLogicOp. **/
		eColorLogicOp = GL_COLOR_LOGIC_OP,

		/** If enabled, cull polygons based on their winding in window coordinates. See glCullFace. **/
		eCullFace = GL_CULL_FACE,

		/** If enabled, debug messages are produced by a debug context. When disabled, the debug message log is silenced.
		 * Note that in a non-debug context, very few, if any messages might be produced, even when GL_DEBUG_OUTPUT is enabled. **/
		eDebugOutput = GL_DEBUG_OUTPUT,

		/** If enabled, debug messages are produced synchronously by a debug context. If disabled, debug messages may be produced asynchronously.
		 * In particular, they may be delayed relative to the execution of GL commands,
		 * and the debug callback function may be called from a thread other than that in which the commands are executed. See glDebugMessageCallback. **/
		eDebugOutputSynchronous = GL_DEBUG_OUTPUT_SYNCHRONOUS,

		/** If enabled, the -wc ≤ zc ≤ wc plane equation is ignored by view volume clipping (effectively, there is no near or far plane clipping).
		 * See glDepthRange. **/
		eDepthClamp = GL_DEPTH_CLAMP,

		/** If enabled, do depth comparisons and update the depth buffer. Note that even if the depth buffer exists and the depth mask is non-zero,
		 * the depth buffer is not updated if the depth test is disabled. See glDepthFunc and glDepthRange. **/
		eDepthTest = GL_DEPTH_TEST,

		/** If enabled, dither color components or indices before they are written to the color buffer. **/
		eDither = GL_DITHER,

		/** If enabled and the value of GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the framebuffer attachment corresponding to the destination buffer is GL_SRGB,
		 * the R, G, and B destination color values (after conversion from fixed-point to floating-point)
		 * are considered to be encoded for the sRGB color space and hence are linearized prior to their use in blending. **/
		eFramebufferSRGB = GL_FRAMEBUFFER_SRGB,

		/** If enabled, draw lines with correct filtering. Otherwise, draw aliased lines. See glLineWidth. **/
		eLineSmooth = GL_LINE_SMOOTH,

		/** If enabled, use multiple fragment samples in computing the final color of a pixel. See glSampleCoverage. **/
		eMultisample = GL_MULTISAMPLE,

		/** If enabled, and if the polygon is rendered in GL_FILL mode,
		 * an offset is added to depth values of a polygon's fragments before the depth comparison is performed. See glPolygonOffset. **/
		ePolygonOffsetFill = GL_POLYGON_OFFSET_FILL,

		/** If enabled, and if the polygon is rendered in GL_LINE mode,
		 * an offset is added to depth values of a polygon's fragments before the depth comparison is performed. See glPolygonOffset. **/
		ePolygonOffsetLine = GL_POLYGON_OFFSET_LINE,

		/** If enabled, an offset is added to depth values of a polygon's fragments before the depth comparison is performed,
		 * if the polygon is rendered in GL_POINT mode. See glPolygonOffset. **/
		ePolygonOffsetPoint = GL_POLYGON_OFFSET_POINT,

		/** If enabled, draw polygons with proper filtering. Otherwise, draw aliased polygons. For correct antialiased polygons,
		 * an alpha buffer is needed and the polygons must be sorted front to back. **/
		ePolygonSmooth = GL_POLYGON_SMOOTH,

		/** Enables primitive restarting. If enabled,
		 * any one of the draw commands which transfers a set of generic attribute array elements to the GL
		 * will restart the primitive when the index of the vertex is equal to the primitive restart index. See glPrimitiveRestartIndex. **/
		ePrimitiveRestart = GL_PRIMITIVE_RESTART,

		/** Enables primitive restarting with a fixed index. If enabled,
		 * any one of the draw commands which transfers a set of generic attribute array elements to the GL
		 * will restart the primitive when the index of the vertex is equal to the fixed primitive index for the specified index type.
		 * The fixed index is equal to $ 2^{n}-1 $ where n is equal to 8 for GL_UNSIGNED_BYTE, 16 for GL_UNSIGNED_SHORT and 32 for GL_UNSIGNED_INT. **/
		ePrimitiveRestartFixedIndex = GL_PRIMITIVE_RESTART_FIXED_INDEX,

		/** If enabled, all primitives are discarded before rasterization, but after any optional transform feedback.
		 * Also causes glClear and glClearBuffer commands to be ignored. **/
		eRasterizerDiscard = GL_RASTERIZER_DISCARD,

		/** If enabled, compute a temporary coverage value where each bit is determined by the alpha value at the corresponding sample location.
		 * The temporary coverage value is then ANDed with the fragment coverage value. **/
		eSampleAlphaToCoverage = GL_SAMPLE_ALPHA_TO_COVERAGE,

		/** If enabled, each sample alpha value is replaced by the maximum representable alpha value. **/
		eSampleAlphaToOne = GL_SAMPLE_ALPHA_TO_ONE,

		/** If enabled, the fragment's coverage is ANDed with the temporary coverage value. If GL_SAMPLE_COVERAGE_INVERT is set to GL_TRUE,
		 * invert the coverage value. See glSampleCoverage. **/
		eSampleCoverage = GL_SAMPLE_COVERAGE,

		/** If enabled, the active fragment shader is run once for each covered sample,
		 * or at fraction of this rate as determined by the current value of GL_MIN_SAMPLE_SHADING_VALUE. See glMinSampleShading. **/
		eSampleShading = GL_SAMPLE_SHADING,

		/** If enabled, the sample coverage mask generated for a fragment during rasterization will be ANDed with the
		 * value of GL_SAMPLE_MASK_VALUE before shading occurs. See glSampleMaski. **/
		eSampleMask = GL_SAMPLE_MASK,

		/** If enabled, discard fragments that are outside the scissor rectangle. See glScissor. **/
		eScissorTest = GL_SCISSOR_TEST,

		/** If enabled, do stencil testing and update the stencil buffer. See glStencilFunc and glStencilOp.**/
		eStencilTest = GL_STENCIL_TEST,

		/** If enabled, cubemap textures are sampled such that when linearly sampling from the border between two adjacent faces,
		 * texels from both faces are used to generate the final sample value. When disabled,
		 * texels from only a single face are used to construct the final sample value. **/
		eTextureCubeMapSeamless = GL_TEXTURE_CUBE_MAP_SEAMLESS,

		/** If enabled and a vertex or geometry shader is active,
		 * then the derived point size is taken from the (potentially clipped) shader builtin gl_PointSize and clamped
		 * to the implementation-dependent point size range. **/
		eProgramPointSize = GL_PROGRAM_POINT_SIZE
	};

	enum class ProgramQuery : Enum
	{
		/** gl::getProgramiv returns TRUE if the program is currently flagged for deletion, and FALSE otherwise. **/
		eDeleteStatus = GL_DELETE_STATUS,

		/** gl::getProgramiv returns GL_TRUE if the last link operation on the program was successful, and GL_FALSE otherwise. **/
		eLinkStatus = GL_LINK_STATUS,

		/** gl::getProgramiv returns GL_TRUE or if the last validation operation on the program was successful, and GL_FALSE otherwise. **/
		eValidateStatus = GL_VALIDATE_STATUS,

		/** gl::getProgramiv returns the number of characters in the information log for the program including the null termination character
		 * (i.e., the size of the character buffer required to store the information log). If the program has no information log, a value of 0 is returned. **/
		eInfoLogLength = GL_INFO_LOG_LENGTH,

		/** gl::getProgramiv returns the number of shader objects attached to the program. **/
		eAttachedShaders = GL_ATTACHED_SHADERS,

		/** gl::getProgramiv returns the number of active attribute atomic counter buffers used by the program. **/
		eActiveAtomicCounterBuffers = GL_ACTIVE_ATOMIC_COUNTER_BUFFERS,

		/** gl::getProgramiv returns the number of active attribute variables for the program. **/
		eActiveAttributes = GL_ACTIVE_ATTRIBUTES,

		/** gl::getProgramiv returns the length of the longest active attribute name for the program,
		 * including the null termination character (i.e., the size of the character buffer required to store the longest attribute name).
		 * If no active attributes exist, 0 is returned. **/
		eActiveAttributeMaxLength = GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,

		/** gl::getProgramiv returns the number of active uniform variables for the program. **/
		eActiveUniforms = GL_ACTIVE_UNIFORMS,

		/** gl::getProgramiv returns the length of the longest active uniform variable name for the program,
		 * including the null termination character (i.e., the size of the character buffer required to store the longest uniform variable name).
		 * If no active uniform variables exist, 0 is returned. **/
		eActiveUniformMaxLength = GL_ACTIVE_UNIFORM_MAX_LENGTH,

		/** gl::getProgramiv returns the length of the program binary, in bytes that will be returned by a call to glGetProgramBinary.
		 * When a progam's GL_LINK_STATUS is GL_FALSE, its program binary length is zero. **/
		eProgramBinaryLength = GL_PROGRAM_BINARY_LENGTH,

		/** gl::getProgramiv returns an array of three integers containing the local work group size of the compute program as specified by its input layout qualifier(s).
		 * the program must be the name of a program object that has been previously linked successfully and contains a binary for the compute shader stage. **/
		eComputeWorkGroupSize = GL_COMPUTE_WORK_GROUP_SIZE,

		/** gl::getProgramiv returns a symbolic constant indicating the buffer mode used when transform feedback is active.
		 * This may be GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS. **/
		eTransformFeedbackBufferMode = GL_TRANSFORM_FEEDBACK_BUFFER_MODE,

		/** gl::getProgramiv returns the number of varying variables to capture in transform feedback mode for the program. **/
		eTransformFeedbackVaryings = GL_TRANSFORM_FEEDBACK_VARYINGS,

		/** gl::getProgramiv returns the length of the longest variable name to be used for transform feedback, including the null-terminator. **/
		eTransformFeedbackVaryingMaxLength = GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH,

		/** gl::getProgramiv returns the maximum number of vertices that the geometry shader in the program will output. **/
		eGeometryVerticesOut = GL_GEOMETRY_VERTICES_OUT,

		/** gl::getProgramiv returns a symbolic constant indicating the primitive type accepted as input to the geometry shader contained in the program. **/
		eGeometryInputType = GL_GEOMETRY_INPUT_TYPE,

		/** gl::getProgramiv returns a symbolic constant indicating the primitive type that will be output by the geometry shader contained in the program. **/
		eGeometryOutputType = GL_GEOMETRY_OUTPUT_TYPE,
	};

	enum class ShaderBinaryFormat : Enum
	{
		eSpirV = GL_SHADER_BINARY_FORMAT_SPIR_V
	};

	enum class TextureType : Enum
	{
		/** Images in this texture all are 1-dimensional. They have width, but no height or depth. **/
		e1D = GL_TEXTURE_1D,
		/** Images in this texture all are 2-dimensional. They have width and height, but no depth. **/
		e2D = GL_TEXTURE_2D,
		/** Images in this texture all are 3-dimensional. They have width, height, and depth. **/
		e3D = GL_TEXTURE_3D,
		/** The image in this texture (only one image. No mipmapping) is 2-dimensional. Texture coordinates used for these textures are not normalized. **/
		eRectangle = GL_TEXTURE_RECTANGLE,
		/** The image in this texture (only one image. No mipmapping) is 1-dimensional. The storage for this data comes from a Buffer Object. **/
		eBuffer = GL_TEXTURE_BUFFER,
		/** There are exactly 6 distinct sets of 2D images, each image being of the same size and must be of a square size. These images act as 6 faces of a cube. **/
		eCubeMap = GL_TEXTURE_CUBE_MAP,
		/** Images in this texture all are 1-dimensional. However, it contains multiple sets of 1-dimensional images, all within one texture. The array length is part of the texture's size. **/
		e1DArray = GL_TEXTURE_1D_ARRAY,
		/** Images in this texture all are 2-dimensional. However, it contains multiple sets of 2-dimensional images, all within one texture. The array length is part of the texture's size. **/
		e2DArray = GL_TEXTURE_2D_ARRAY,
		/** Images in this texture are all cube maps. It contains multiple sets of cube maps, all within one texture. The array length * 6 (number of cube faces) is part of the texture size. **/
		eCubeMapArray = GL_TEXTURE_CUBE_MAP_ARRAY,
		/** The image in this texture (only one image. No mipmapping) is 2-dimensional. Each pixel in these images contains multiple samples instead of just one value. **/
		e2DMultisample = GL_TEXTURE_2D_MULTISAMPLE,
		/** Combines 2D array and 2D multisample types. No mipmapping. **/
		e2DMultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY
	};
}
