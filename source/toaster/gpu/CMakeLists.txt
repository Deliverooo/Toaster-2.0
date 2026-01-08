set(GPU_SRC

		gpu_context.cpp
		gpu_context.hpp

		texture.cpp
		texture.hpp

		shader.cpp
		shader.hpp

		shader_reflection.cpp
		shader_reflection.hpp

		shader_compiler.cpp
		shader_compiler.hpp

		shader_common.hpp

		swapchain.cpp
		swapchain.hpp

		pipeline.cpp
		pipeline.hpp

		framebuffer.cpp
		framebuffer.hpp

		vertex_buffer.cpp
		vertex_buffer.hpp

		index_buffer.cpp
		index_buffer.hpp
)

add_library(toast_gpu STATIC)

target_include_directories(toast_gpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(toast_gpu PRIVATE ${GPU_SRC})

target_link_libraries(toast_gpu PUBLIC tst::toast_lib)
target_link_libraries(toast_gpu PUBLIC glfw)
target_link_libraries(toast_gpu PUBLIC stb)

target_link_libraries(toast_gpu PUBLIC nvrhi)
target_link_libraries(toast_gpu PUBLIC nvrhi_vk)

target_link_libraries(toast_gpu PUBLIC Vulkan::Vulkan)
target_link_libraries(toast_gpu PUBLIC Vulkan::shaderc_combined)

target_include_directories(toast_gpu PUBLIC "$ENV{VULKAN_SDK}/Include")

target_link_libraries(toast_gpu PUBLIC
		debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-cored.lib"
		optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-core.lib"

		debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-glsld.lib"
		optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-glsl.lib"

		debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-reflectd.lib"
		optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-reflect.lib"
)

add_library(tst::toast_gpu ALIAS toast_gpu)