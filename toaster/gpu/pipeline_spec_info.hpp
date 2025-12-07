#pragma once

#include "shader.hpp"
#include "framebuffer.hpp"
#include "vertex_buffer_layout.hpp"

namespace tst
{
	enum class EPrimitiveTopology
	{
		eNone = 0,
		ePoints,
		eLines,
		eTriangles,
		eLineStrip,
		eTriangleStrip,
		eTriangleFan
	};

	enum class EDepthCompareOperator
	{
		eNone = 0,
		eNever,
		eNotEqual,
		eLess,
		eLessOrEqual,
		eGreater,
		eGreaterOrEqual,
		eEqual,
		eAlways
	};

	struct PipelineSpecInfo
	{
		RefPtr<Shader>        shader;
		RefPtr<Framebuffer>   targetFramebuffer;
		VertexBufferLayout    layout;
		VertexBufferLayout    instanceLayout;
		VertexBufferLayout    boneInfluenceLayout;
		EPrimitiveTopology    topology        = EPrimitiveTopology::eTriangles;
		EDepthCompareOperator depthOperator   = EDepthCompareOperator::eGreaterOrEqual;
		bool                  backfaceCulling = true;
		bool                  depthTest       = true;
		bool                  depthWrite      = true;
		bool                  wireframe       = false;
		float                 lineWidth       = 1.0f;

		std::string debugName;
	};

	// Identical to Vulkan's VkPipelineStageFlagBits
	// Note: this is a bitfield
	enum class PipelineStage
	{
		None                        = 0,
		TopOfPipe                   = 0x00000001,
		DrawIndirect                = 0x00000002,
		VertexInput                 = 0x00000004,
		VertexShader                = 0x00000008,
		TesselationControlShader    = 0x00000010,
		TesselationEvaluationShader = 0x00000020,
		GeometryShader              = 0x00000040,
		FragmentShader              = 0x00000080,
		EarlyFragmentTests          = 0x00000100,
		LateFragmentTests           = 0x00000200,
		ColorAttachmentOutput       = 0x00000400,
		ComputeShader               = 0x00000800,
		Transfer                    = 0x00001000,
		BottomOfPipe                = 0x00002000,
		Host                        = 0x00004000,
		AllGraphics                 = 0x00008000,
		AllCommands                 = 0x00010000
	};

	// Identical to Vulkan's VkAccessFlagBits
	// Note: this is a bitfield
	enum class EResourceAccessFlags
	{
		eNone                        = 0,
		eIndirectCommandRead         = 0x00000001,
		eIndexRead                   = 0x00000002,
		eVertexAttributeRead         = 0x00000004,
		eUniformRead                 = 0x00000008,
		eInputAttachmentRead         = 0x00000010,
		eShaderRead                  = 0x00000020,
		eShaderWrite                 = 0x00000040,
		eColorAttachmentRead         = 0x00000080,
		eColorAttachmentWrite        = 0x00000100,
		eDepthStencilAttachmentRead  = 0x00000200,
		eDepthStencilAttachmentWrite = 0x00000400,
		eTransferRead                = 0x00000800,
		eTransferWrite               = 0x00001000,
		eHostRead                    = 0x00002000,
		eHostWrite                   = 0x00004000,
		eMemoryRead                  = 0x00008000,
		eMemoryWrite                 = 0x00010000
	};
}
