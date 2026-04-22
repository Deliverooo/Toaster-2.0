#include <iostream>

#include "client_application.hpp"
#include "toast_gpu/vk/vk_command_buffer.hpp"
#include "toast_gpu/vk/vk_compute_pass.hpp"
#include "toast_gpu/vk/vk_compute_pipeline.hpp"

#include "toast_gpu/vk/vk_gpu_context.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_gpu/vk/vk_pipeline.hpp"
#include "toast_gpu/vk/vk_shader.hpp"
#include "toast_gpu/vk/vk_shader_compiler.hpp"
#include "toast_render/renderer.hpp"

auto cstringArrayToVector(toaster::CString *p_arr, uint32 p_size) -> std::vector<toaster::CString>
{
	std::vector<toaster::CString> vec{p_size};
	for (uint32 i{0u}; i < p_size; ++i)
		vec.emplace_back(p_arr[i]);
	return vec;
}

#if USE_WINMAIN
INT WINAPI WinMain([[maybe_unused]] HINSTANCE hInstance, [[maybe_unused]] HINSTANCE hPrevInstance, [[maybe_unused]] LPSTR lpCmdLine, [[maybe_unused]] INT nCmdShow)
{
#else
int32 main(int32 p_argc, char **p_argv) // Maybe_todo, Forward these parameters to the application for it to handle
{
	#endif

	uint32                                         extension_count{0u};
	auto                                           required_extensions{cstringArrayToVector(glfwGetRequiredInstanceExtensions(&extension_count), extension_count)};
	toaster::gpu::VKInstanceSpecInfo::ExtensionSet instance_extensions{required_extensions.begin(), required_extensions.end()};

	instance_extensions.insert(vk::KHRSurfaceExtensionName);
	toaster::gpu::VKInstanceSpecInfo vk_instance_spec_info{};
	vk_instance_spec_info.appName            = "Toaster-2.0 -> Vulkan QT";
	vk_instance_spec_info.requiredExtensions = instance_extensions;
	toaster::gpu::VKInstance vk_instance{vk_instance_spec_info};

	toaster::gpu::VKPhysicalDeviceSpecInfo vk_physical_device_spec_info{};
	vk_physical_device_spec_info.requiredExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRDynamicRenderingExtensionName,
		vk::KHRTimelineSemaphoreExtensionName,
		vk::EXTCustomBorderColorExtensionName,
		vk::KHRMaintenance6ExtensionName,
		vk::KHRLoadStoreOpNoneExtensionName
	};
	toaster::gpu::VKPhysicalDevice vk_physical_device{&vk_instance, vk_physical_device_spec_info};

	toaster::gpu::VKLogicalDeviceSpecInfo vk_logical_device_spec_info{};
	vk_logical_device_spec_info.surface            = nullptr;
	vk_logical_device_spec_info.requiredExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRDynamicRenderingExtensionName,
		vk::KHRTimelineSemaphoreExtensionName,
		vk::EXTCustomBorderColorExtensionName,
		vk::KHRMaintenance6ExtensionName,
		vk::KHRLoadStoreOpNoneExtensionName
	};

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> feature_chain{{}, {}, {}, {}};
	feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy                 = true;
	feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading                 = true;
	feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid                  = true;
	feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                   = true;
	feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                    = true;
	feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2                    = true;
	feature_chain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;
	vk_logical_device_spec_info.pNext                                                           = feature_chain.get<vk::PhysicalDeviceFeatures2>();
	toaster::gpu::VKLogicalDevice vk_logical_device{&vk_physical_device, vk_logical_device_spec_info};

	toaster::gpu::VKGPUContextSpecInfo gpu_context_spec_info{};
	toaster::gpu::VKGPUContext         gpu_context{&vk_logical_device, gpu_context_spec_info};

	{
		auto storage_buffer{toaster::make_reference<toaster::gpu::VKStorageBuffer>(&gpu_context, sizeof(int32))};

		auto cs_shader{
			toaster::gpu::VKShaderCompiler::compileToShaderFromPaths(&gpu_context,
																	 {{vk::ShaderStageFlagBits::eCompute, "../source/toaster/toast_shaders/test.comp.glsl"}},
																	 "Compute_Test")
		};

		auto compute_pipeline{toaster::make_reference<toaster::gpu::VKComputePipeline>(&gpu_context, cs_shader)};
		auto compute_pass{toaster::make_reference<toaster::gpu::VKComputePass>(&gpu_context, compute_pipeline)};
		compute_pass->setInput("Test", storage_buffer);
		compute_pass->bake();

		toaster::gpu::VKCommandBuffer command_buffer{&gpu_context, vk::QueueFlagBits::eCompute};

		command_buffer.begin();
		toaster::Renderer::beginCompute(command_buffer.getVulkanCommandBuffer(), 0, compute_pass);
		toaster::Renderer::dispatchCompute(command_buffer.getVulkanCommandBuffer(), 0, compute_pass, nullptr, 1, 1, 1);
		toaster::Renderer::endCompute(command_buffer.getVulkanCommandBuffer(), 0, compute_pass);
		command_buffer.end();
		command_buffer.submit();

		int32 data{0};
		void *mapped{storage_buffer->mapMemory(0, sizeof(int32))};
		std::memcpy(&data, mapped, sizeof(int32));
		storage_buffer->unmapMemory();
		LOG_INFO("{}", data);
	}

	std::cin.get();
}
