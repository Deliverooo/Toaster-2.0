#include "toast_gpu/device.hpp"

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

		m_deletionQueue = makeUnique<DeletionQueue>(p_desc.numDeletionQueues);
	}

	Device::~Device()
	{
		m_deletionQueue->executeAll();
		m_deletionQueue.reset();

		m_allocator.reset();

		m_device.reset();
		m_physicalDevice.reset();
		m_instance.reset();
	}

	auto Device::performGarbageCollection(uint32 p_queue_index) -> void
	{
		m_deletionQueue->execute();
		m_deletionQueue->setQueueIndex(p_queue_index);
	}

	auto Device::flushDeletionQueue() -> void
	{
		m_deletionQueue->executeAll();
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

	auto Device::waitForTimelineSemaphores(const InitialiserList<const vk::Semaphore> &p_semaphores,
											   const InitialiserList<const uint64> &       p_target_values) const -> void
	{
		vk::SemaphoreWaitInfo semaphore_wait_info{};
		semaphore_wait_info.setSemaphores(p_semaphores);
		semaphore_wait_info.setValues(p_target_values);
		if (m_device->getDevice().waitSemaphores(semaphore_wait_info, INFINITE) != vk::Result::eSuccess)
			TST_PERMA_ASSERT_MSG(false, "Failed to wait for timeline semaphores");
	}
}
