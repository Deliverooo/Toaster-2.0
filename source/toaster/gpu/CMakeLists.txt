set(GPU_SRC

		gpu_context.cpp
		gpu_context.hpp

		texture.cpp
		texture.hpp

		shader.cpp
		shader.hpp

		shader_reflection.cpp
		shader_reflection.hpp

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
target_link_libraries(toast_gpu PRIVATE spirv-cross-core)

if (${Vulkan_FOUND})
	message(NOTICE "Found Vulkan and glslandValidator! :)")
	target_link_libraries(toast_gpu PUBLIC Vulkan::Vulkan)
	target_link_libraries(toast_gpu PUBLIC nvrhi_vk)


else ()
	message(FATAL_ERROR "Vulkan enabled as a graphics backend, but cannot be found!")
endif ()

add_library(tst::toast_gpu ALIAS toast_gpu)