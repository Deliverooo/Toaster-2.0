#include "toast_gpu/device.hpp"

#include "toast_gpu/command_pool.hpp"

#include <d3d12.h>

#include "toast_gpu/command_list.hpp"

namespace toaster::gpu
{
	Device::Device(const DeviceDesc p_desc)
	{
		FunctionDispatcher::initBaseFunctions();
		InstanceDesc instance_desc{};
		instance_desc.enableValidationLayers = p_desc.enableDebugInfo;
		instance_desc.requiredExtensions     = {};
		if (p_desc.usingSwapchain)
		{
			instance_desc.requiredExtensions.emplace(vk::KHRSurfaceExtensionName);
			instance_desc.requiredExtensions.emplace(vk::KHRWin32SurfaceExtensionName);
		}
		instance_desc.applicationName = "Orbo's Exile";
		m_instance                    = makeUnique<Instance>(instance_desc);
		FunctionDispatcher::initInstanceFunctions(m_instance->getInstance());

		PhysicalDeviceDesc physical_device_desc{};
		physical_device_desc.requiredExtensions = {
			vk::EXTDescriptorHeapExtensionName,
			vk::EXTShaderObjectExtensionName,
			vk::KHRMaintenance1ExtensionName,
			vk::KHRShaderUntypedPointersExtensionName,
			vk::KHRMaintenance9ExtensionName
		};
		if (p_desc.usingSwapchain)
			physical_device_desc.requiredExtensions.emplace(vk::KHRSwapchainExtensionName);

		m_physicalDevice = makeUnique<PhysicalDevice>(*m_instance, physical_device_desc);

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceVulkan14Features,
			vk::PhysicalDeviceShaderObjectFeaturesEXT, vk::PhysicalDeviceDescriptorHeapFeaturesEXT, vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR,
			vk::PhysicalDeviceMaintenance9FeaturesKHR> feature_chain{{}, {}, {}, {}, {}, {}, {}, {}};

		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy                        = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.sampleRateShading                        = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fillModeNonSolid                         = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.fragmentStoresAndAtomics                 = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.shaderInt16                              = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.shaderInt64                              = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.vertexPipelineStoresAndAtomics           = true;
		feature_chain.get<vk::PhysicalDeviceFeatures2>().features.multiDrawIndirect                        = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().scalarBlockLayout                          = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore                          = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().bufferDeviceAddress                        = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().runtimeDescriptorArray                     = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderInt8                                 = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().storagePushConstant8                       = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().uniformAndStorageBuffer8BitAccess          = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().storageBuffer8BitAccess                    = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderStorageBufferArrayNonUniformIndexing = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderUniformBufferArrayNonUniformIndexing = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderSampledImageArrayNonUniformIndexing  = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderUniformBufferArrayNonUniformIndexing = true;
		feature_chain.get<vk::PhysicalDeviceVulkan12Features>().shaderStorageImageArrayNonUniformIndexing  = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering                           = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2                           = true;
		feature_chain.get<vk::PhysicalDeviceVulkan13Features>().maintenance4                               = true;
		feature_chain.get<vk::PhysicalDeviceVulkan14Features>().indexTypeUint8                             = true;
		feature_chain.get<vk::PhysicalDeviceVulkan14Features>().maintenance6                               = true;
		feature_chain.get<vk::PhysicalDeviceShaderObjectFeaturesEXT>().shaderObject                        = true;
		feature_chain.get<vk::PhysicalDeviceDescriptorHeapFeaturesEXT>().descriptorHeap                    = true;
		feature_chain.get<vk::PhysicalDeviceShaderUntypedPointersFeaturesKHR>().shaderUntypedPointers      = true;
		feature_chain.get<vk::PhysicalDeviceMaintenance9FeaturesKHR>().maintenance9                        = true;

		LogicalDeviceDesc logical_device_desc{};
		logical_device_desc.pNextDeviceFeatures = feature_chain.get<vk::PhysicalDeviceFeatures2>();
		logical_device_desc.enabledExtensions   = physical_device_desc.requiredExtensions;
		m_device                                = makeUnique<LogicalDevice>(*m_physicalDevice, logical_device_desc);
		FunctionDispatcher::initDeviceFunctions(m_device->getDevice());

		m_allocator = makeUnique<Allocator>(*m_instance, *m_physicalDevice, *m_device);
	}

	Device::~Device()
	{
		m_allocator.reset();

		m_device.reset();
		m_physicalDevice.reset();
		m_instance.reset();
	}

	auto Device::createCommandPool(EQueueType p_queue_type, ECommandPoolFlags p_pool_flags) -> CommandPool
	{
		uint32 queue_family_index{UINT32_MAX};
		switch (p_queue_type)
		{
			case EQueueType::eGraphics: queue_family_index = m_device->getQueueFamilyIndices().graphics;
				break;
			case EQueueType::eCompute: queue_family_index = m_device->getQueueFamilyIndices().compute;
				break;
			case EQueueType::eTransfer: queue_family_index = m_device->getQueueFamilyIndices().transfer;
				break;
		}

		vk::CommandPoolCreateFlags flags{0u};
		if (p_pool_flags & ECommandPoolBits::eReset)
			flags |= vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		if (p_pool_flags & ECommandPoolBits::eTransient)
			flags |= vk::CommandPoolCreateFlagBits::eTransient;

		vk::CommandPoolCreateInfo command_pool_create_info{};
		command_pool_create_info.queueFamilyIndex = queue_family_index;
		command_pool_create_info.flags            = flags;

		vk::CommandPool command_pool{m_device->getDevice().createCommandPool(command_pool_create_info)};

		return CommandPool{*this, command_pool, p_pool_flags};
	}

	auto Device::executeCommandLists(const InitialiserList<const CommandList> &p_command_lists, const InitialiserList<const vk::SemaphoreSubmitInfo> &p_wait_semaphores,
									 const InitialiserList<const vk::SemaphoreSubmitInfo> &p_signal_semaphores, vk::Fence p_signal_fence) const -> void
	{
		std::vector<vk::CommandBufferSubmitInfo> command_buffer_infos;
		for (const auto &cmd: p_command_lists)
			command_buffer_infos.emplace_back(cmd.getCommandBuffer());

		vk::SubmitInfo2 submit_info{};
		submit_info.setCommandBufferInfos(command_buffer_infos);
		submit_info.setWaitSemaphoreInfos(p_wait_semaphores);
		submit_info.setSignalSemaphoreInfos(p_signal_semaphores);

		m_device->getGraphicsQueue().submit2(submit_info, p_signal_fence);
	}

	auto Device::createTimelineSemaphore(uint64 p_initial_value) const -> vk::Semaphore
	{
		vk::SemaphoreTypeCreateInfo timeline_semaphore_create_info{};
		timeline_semaphore_create_info.initialValue  = p_initial_value;
		timeline_semaphore_create_info.semaphoreType = vk::SemaphoreType::eTimeline;

		vk::SemaphoreCreateInfo create_info{};
		create_info.pNext = &timeline_semaphore_create_info;

		return m_device->getDevice().createSemaphore(create_info);
	}

	auto Device::waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores, const InitialiserList<const uint64> &p_target_values) const -> void
	{
		vk::SemaphoreWaitInfo semaphore_wait_info{};
		semaphore_wait_info.setSemaphores(p_semaphores);
		semaphore_wait_info.setValues(p_target_values);
		if (m_device->getDevice().waitSemaphores(semaphore_wait_info, INFINITE) != vk::Result::eSuccess)
			TST_PERMA_ASSERT_MSG(false, "Failed to wait for timeline semaphores");
	}

	auto Device::waitIdle() const -> void
	{
		m_device->getDevice().waitIdle();
	}
}
