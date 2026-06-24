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
			uintptr cameraBDA; // Ts requires you to have a camera ubo to pass to the pass...
			uint32  samplerAddressOffset;
			uint32  environmentMapAddressOffset;
		};

		SkyboxPass(RenderContext &p_render_ctx, bool32 p_msaa = false); // Only use if you intend on using an override rendering info!
		SkyboxPass(RenderContext &p_render_ctx, tsm::uint2 p_initial_viewport_size, bool32 p_msaa = false);
		SkyboxPass(RenderContext &p_render_ctx, tsm::uint2 p_initial_viewport_size, const ImageHandle &p_environment_map, bool32 p_msaa = false);

		auto getOutputImage() const -> const ImageHandle &;

		auto onRender(gpu::CommandBuffer &p_cmd, uintptr p_camera_bda_ptr) const -> void;
		auto onRender(gpu::CommandBuffer &p_cmd, uintptr p_camera_bda_ptr, const RenderingInfo &p_rendering_info) const -> void;
		auto onResize(tsm::uint2 p_size) -> void;

		auto setEnvironmentMap(const ImageHandle &p_environment_map) -> void;

	private:
		auto _construct(bool32 p_msaa) -> void;

		tsm::uint2 m_viewportSize{UINT32_MAX};

		GraphicsStateUnique m_skyboxState{nullptr};
		ImageHandle         m_renderTargetImage{nullptr};

		ImageHandle m_environmentMap{nullptr};
	};
}
