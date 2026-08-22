#include <toast_gpu/descriptor_heap.hpp>

#include "toast_gpu/buffer.hpp"
#include "toast_gpu/shader.hpp"
#include "toast_gpu/texture.hpp"

#include "triangle.vert.h"

using namespace toaster;

auto main(TST_UNUSED int32 p_argc, TST_UNUSED char **p_argv) -> int32
{
	#if 1
	gpu::FunctionDispatcher::initBaseFunctions();
	gpu::InstanceDesc instance_desc{};
	instance_desc.enableValidationLayers = true;
	instance_desc.applicationName        = "Test app";
	gpu::Instance gpu_instance{instance_desc};
	gpu::FunctionDispatcher::initInstanceFunctions(gpu_instance.getInstance());

	gpu::PhysicalDeviceDesc physical_device_desc{};
	physical_device_desc.requiredExtensions = {
		vk::EXTDescriptorHeapExtensionName,
		vk::KHRBufferDeviceAddressExtensionName,
		vk::KHRSynchronization2ExtensionName,
		vk::EXTShaderObjectExtensionName
	};
	gpu::PhysicalDevice physical_device{gpu_instance, physical_device_desc};

	gpu::LogicalDeviceDesc logical_device_desc{};
	logical_device_desc.enabledExtensions = physical_device_desc.requiredExtensions;

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures, vk::PhysicalDeviceDescriptorHeapFeaturesEXT,
		vk::PhysicalDeviceSynchronization2Features, vk::PhysicalDeviceTimelineSemaphoreFeatures, vk::PhysicalDeviceShaderObjectFeaturesEXT> feature_chain{
		{},
		{},
		{},
		{},
		{},
		{}
	};
	feature_chain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>().bufferDeviceAddress = true;
	feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap        = true;
	feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap        = true;
	feature_chain.get<vk::PhysicalDeviceSynchronization2Features>().synchronization2       = true;
	feature_chain.get<vk::PhysicalDeviceTimelineSemaphoreFeatures>().timelineSemaphore     = true;
	feature_chain.get<vk::PhysicalDeviceShaderObjectFeaturesEXT>().shaderObject            = true;
	logical_device_desc.pNextDeviceFeatures                                                = feature_chain.get<vk::PhysicalDeviceFeatures2>();

	gpu::LogicalDevice logical_device{physical_device, logical_device_desc};
	gpu::FunctionDispatcher::initDeviceFunctions(logical_device.getDevice());

	gpu::Allocator              allocator{gpu_instance, physical_device, logical_device};
	gpu::ResourceDescriptorHeap resource_descriptor_heap{logical_device, physical_device, allocator, 32u, 32u};

	TST_PERMA_ASSERT(gpu::FunctionDispatcher::get().vkCreateShadersEXT);
	{
		gpu::ShaderManager shader_manager{logical_device};

		vk::ShaderCreateInfoEXT vertex_shader_create_info{};
		vertex_shader_create_info.pName     = "main";
		vertex_shader_create_info.pCode     = c_triangle_vert_bytecode;
		vertex_shader_create_info.codeSize  = std::size(c_triangle_vert_bytecode) * sizeof(uint32);
		vertex_shader_create_info.codeType  = vk::ShaderCodeTypeEXT::eSpirv;
		vertex_shader_create_info.stage     = vk::ShaderStageFlagBits::eVertex;
		vertex_shader_create_info.nextStage = vk::ShaderStageFlagBits::eFragment;
		vertex_shader_create_info.flags     = vk::ShaderCreateFlagBitsEXT::eDescriptorHeap;

		gpu::ShaderHandle shader{shader_manager.createShader(vertex_shader_create_info)};

		shader_manager.destroyShader(shader);
		// vk::ShaderEXT shader{*logical_device.getDevice().createShaderEXT(vertex_shader_create_info, nullptr, gpu::FunctionDispatcher::get())};

		// logical_device.getDevice().destroyShaderEXT(shader, nullptr, gpu::FunctionDispatcher::get());
		// shader_manager.destroyShader(shader);
	}

	#endif

	return 0;
}
