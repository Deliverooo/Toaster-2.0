set(TST_GPU_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/device_manager.cpp"
)

# Add platform-specific sources
if (TST_ENABLE_D3D11)
	list(APPEND TST_GPU_SOURCES
			"${CMAKE_CURRENT_SOURCE_DIR}/d3d11/d3d11_device_manager.cpp"
	)
endif ()

if (TST_ENABLE_D3D12)
	list(APPEND TST_GPU_SOURCES
			"${CMAKE_CURRENT_SOURCE_DIR}/d3d12/d3d12_device_manager.cpp"
	)
endif ()

if (TST_ENABLE_VULKAN)
	list(APPEND TST_GPU_SOURCES
			"${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_swapchain.cpp"
			"${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_device_manager.cpp"
	)
endif ()

target_sources(ToasterEngine PRIVATE ${TST_GPU_SOURCES})