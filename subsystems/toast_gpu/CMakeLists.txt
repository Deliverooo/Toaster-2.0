set(GPU_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_instance.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_physical_device.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_logical_device.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_command_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_swapchain.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_pipeline.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_vertex_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_index_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_uniform_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_shader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_shader_compiler.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_common.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_raw_image.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_storage_image.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_texture.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_compute_pipeline.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/vk/vk_storage_buffer.cpp
)

add_library(ToastGPU SHARED)

target_include_directories(ToastGPU PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
		$<INSTALL_INTERFACE:include/toast_gpu>)

target_sources(ToastGPU PRIVATE ${GPU_SRC})

target_link_libraries(ToastGPU PUBLIC Toaster::ToastLib)
target_link_libraries(ToastGPU PUBLIC stb)

target_link_libraries(ToastGPU PUBLIC Vulkan::Vulkan)
target_link_libraries(ToastGPU PUBLIC Vulkan::shaderc_combined)

target_include_directories(ToastGPU PUBLIC "$ENV{VULKAN_SDK}/Include")

target_link_libraries(ToastGPU PUBLIC
		debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-cored.lib"
		optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-core.lib"

		debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-glsld.lib"
		optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-glsl.lib"

		debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-reflectd.lib"
		optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-reflect.lib"
)

add_library(Toaster::ToastGPU ALIAS ToastGPU)

add_compile_definitions(ToastGPU PUBLIC TST_GPU_BUILD_DLL)
add_compile_definitions(ToastGPU PRIVATE TST_GPU_DLL_EXPORT)

install(TARGETS ToastGPU
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)