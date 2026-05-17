#include <iostream>

#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_render/render_context.hpp"

auto main(int32 p_argc, char **p_argv) -> int32
{
	toaster::render::RenderContextSpecInfo render_context_spec_info{};
	render_context_spec_info.binaryDir      = p_argv[0];
	render_context_spec_info.printDebugInfo = true;
	render_context_spec_info.createGlobals  = false;
	toaster::render::RenderContext render_context{render_context_spec_info};

	toaster::gpu::ShaderCompiler shader_compiler{render_context.getLogicalDevice()};
	auto                         shader{shader_compiler.compileToShaderFromPaths({vk::ShaderStageFlagBits::eCompute}, {"../resources/shaders/test.comp.glsl"})};

	auto compute_pipeline{render_context.createGPU<toaster::gpu::VKComputePipeline>(shader)};
	auto compute_pass{render_context.createGPU<toaster::gpu::VKComputePass>(compute_pipeline)};

	auto storage_buffer{render_context.createGPU<toaster::gpu::VKStorageBuffer>(sizeof(int32))};
	compute_pass->setInput("StorageBuffer", storage_buffer);
	compute_pass->bake();

	auto command_buffer{render_context.createGPU<toaster::gpu::VKCommandBuffer>(vk::QueueFlagBits::eCompute)};
	command_buffer->begin();
	render_context.beginCompute(command_buffer, 0, compute_pass);
	render_context.dispatchCompute(command_buffer, 0, compute_pass, nullptr, 1, 1, 1);
	command_buffer->endAndSubmit();

	auto storage{storage_buffer->getStorage<int32>()};
	LOG_INFO("{}", storage);

	render_context.performGarbageCollection();
	std::cin.get();
	return 0;
}
