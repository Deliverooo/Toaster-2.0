set(GPU_SRC

		gpu_context.cpp
		gpu_context.hpp

		shader_common.hpp

		shader.cpp
		shader.hpp

		texture.hpp
		texture.cpp

		image.hpp

		framebuffer.cpp
		framebuffer.hpp

		gpu_api.hpp
		gpu_api.cpp

		vertex_array.cpp
		vertex_array.hpp

		vertex_buffer.cpp
		vertex_buffer.hpp

		index_buffer.cpp
		index_buffer.hpp
)

if (${WITH_OPENGL_BACKEND})
	list(APPEND GPU_SRC

			gl/gl_gpu_api.hpp
			gl/gl_gpu_api.cpp

			gl/gl_gpu_context.hpp
			gl/gl_gpu_context.cpp

			gl/gl_framebuffer.cpp
			gl/gl_framebuffer.hpp

			gl/gl_vertex_array.cpp
			gl/gl_vertex_array.hpp

			gl/gl_vertex_buffer.cpp
			gl/gl_vertex_buffer.hpp

			gl/gl_index_buffer.cpp
			gl/gl_index_buffer.hpp

			gl/gl_shader.cpp
			gl/gl_shader.hpp

			gl/gl_texture.cpp
			gl/gl_texture.hpp
	)
endif ()

if (${WITH_VULKAN_BACKEND})
	list(APPEND GPU_SRC
			vk/vk_gpu_context.hpp
			vk/vk_gpu_context.cpp

			vk/vk_swapchain.cpp
			vk/vk_swapchain.hpp

			vk/vk_pipeline.cpp
			vk/vk_pipeline.hpp

			vk/vk_vertex_buffer.cpp
			vk/vk_vertex_buffer.hpp

			vk/vk_index_buffer.cpp
			vk/vk_index_buffer.hpp

			vk/vk_shader.cpp
			vk/vk_shader.hpp

			vk/vk_shader_resources.hpp

			vk/vk_image.cpp
			vk/vk_image.hpp

			vk/vk_texture.cpp
			vk/vk_texture.hpp
	)
endif ()

add_library(toast_gpu STATIC)

target_include_directories(toast_gpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_sources(toast_gpu PRIVATE ${GPU_SRC})

target_link_libraries(toast_gpu PUBLIC tst::toast_lib)
target_link_libraries(toast_gpu PUBLIC glfw)
target_link_libraries(toast_gpu PUBLIC stb)

if (${WITH_OPENGL_BACKEND})
	target_link_libraries(toast_gpu PUBLIC opengl-hpp)
	target_link_libraries(toast_gpu PRIVATE Vulkan::shaderc_combined)

	target_include_directories(toast_gpu PUBLIC "$ENV{VULKAN_SDK}/Include")

	target_link_libraries(toast_gpu PUBLIC
			debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-cored.lib"
			optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-core.lib"

			debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-glsld.lib"
			optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-glsl.lib"

			debug "$ENV{VULKAN_SDK}/Lib/spirv-cross-reflectd.lib"
			optimized "$ENV{VULKAN_SDK}/Lib/spirv-cross-reflect.lib"
	)
endif ()

if (${WITH_VULKAN_BACKEND})
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
endif ()

add_library(tst::toast_gpu ALIAS toast_gpu)

add_dependencies(toast_gpu tst::toast_shaders)