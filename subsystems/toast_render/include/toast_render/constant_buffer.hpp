#pragma once

#include "toast_render/shader_library.hpp"

namespace toaster::render
{
	class RenderContext;

	class TST_RENDER_API ConstantBuffer
	{
		TST_RENDER_OBJECT
	public:
		ConstantBuffer(RenderContext& p_render_ctx);

		// auto setUniform()
	private:
	};
}
