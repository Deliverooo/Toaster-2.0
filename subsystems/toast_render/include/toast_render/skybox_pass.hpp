#pragma once

#include "graphics_state.hpp"
#include "image.hpp"
#include "render_attachment.hpp"

namespace toaster::render
{
	// Yes, I am using a mesh shader for the skybox, despite it only rendering 4 vertices
	class TST_RENDER_API SkyboxPass
	{
		TST_RENDER_OBJECT
	public:
		TST_PUSH_CONSTANT_BLOCK(SkyboxConstants)
		{
			// The camera should follow the exact data format of Globals::CameraUB, not Globals::ViewProjCameraUB!!!
			uintptr vertexBufferBDA;
			uintptr cameraBDA; // Ts requires you to have a camera ubo to pass to the pass...
			uint32  samplerAddressOffset;
			uint32  environmentMapAddressOffset;
		};

		SkyboxPass(RenderContext &p_render_ctx);
		SkyboxPass(RenderContext &p_render_ctx, const ImageHandle &p_environment_map);

		auto onRender(gpu::CommandBuffer &p_cmd, const RenderingInfo &p_rendering_info, uintptr p_camera_bda_ptr) const -> void;

		auto setEnvironmentMap(const ImageHandle &p_environment_map) -> void;

	private:
		auto _construct() -> void;

		GraphicsStateUnique m_skyboxState{nullptr};

		ImageHandle m_environmentMap{nullptr};
	};
}
