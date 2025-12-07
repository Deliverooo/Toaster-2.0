set(TST_GPU_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/device_manager.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/command_buffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/vertex_buffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/index_buffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/image.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/compute_pass.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/compute_pipeline.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/material.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/uniform_buffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/uniform_buffer_set.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/storage_buffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/storage_buffer_set.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/framebuffer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/pipeline.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/render_pass.cpp"
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
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_swapchain.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/descriptor_set_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_device_manager.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_shader.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/shader_compiler/vulkan_shader_compiler.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/shader_compiler/vulkan_shader_cache.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/shader_compiler/shader_preprocessing/glsl_includer.cpp
			${CMAKE_CURRENT_SOURCE_DIR}/vulkan/shader_compiler/shader_preprocessing/hlsl_includer.cpp
	)
endif ()

target_sources(ToasterEngine PRIVATE ${TST_GPU_SOURCES})