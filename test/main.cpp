#include <toast_gpu/device.hpp>
#include <toast_gpu/material_manager.hpp>
#include <toast_gpu/command_list.hpp>

using namespace toaster;

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	gpu::DeviceDesc device_desc{};
	device_desc.enableDebugInfo   = true;
	device_desc.usingSwapchain    = false;
	device_desc.numDeletionQueues = 3u;
	gpu::Device device{device_desc};

	// gpu::ShaderDesc vertex_shader_desc{};
	// vertex_shader_desc.code      = c_triangle_vert_bytecode;
	// vertex_shader_desc.codeSize  = std::size(c_triangle_vert_bytecode) * sizeof(uint32);
	// vertex_shader_desc.stage     = gpu::EShaderStage::eVertex;
	// vertex_shader_desc.nextStage = gpu::EShaderStage::ePixel;
	// gpu::ShaderHandle vertex_shader{device.createShader(vertex_shader_desc)};
	//
	// auto cmd{device.createCommandList()};
	//
	// cmd.begin();
	//
	// cmd.bindShaders(vertex_shader);
	//
	// cmd.end();
	//
	// device.executeCommandLists(cmd);
	// device.getDevice().getDevice().waitIdle();
	//
	// device.destroyShader(vertex_shader);


	return 0;
}
