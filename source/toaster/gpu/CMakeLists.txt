set(GPU_SRC

		gpu_context.cpp
		gpu_context.hpp

		shader_common.hpp

		shader.cpp
		shader.hpp

		material.hpp
		material.cpp

		texture.hpp
		texture.cpp

		vertex_array.cpp
		vertex_array.hpp

		vertex_buffer.cpp
		vertex_buffer.hpp

		index_buffer.cpp
		index_buffer.hpp

		globals.cpp
		globals.hpp

		uniform_buffer.cpp
		uniform_buffer.hpp

		mesh.hpp
		mesh.cpp
)

if (${WITH_OPENGL_BACKEND})
	list(APPEND GPU_SRC
			gl/gl_gpu_context.hpp
			gl/gl_gpu_context.cpp

			gl/gl_vertex_array.cpp
			gl/gl_vertex_array.hpp

			gl/gl_vertex_buffer.cpp
			gl/gl_vertex_buffer.hpp

			gl/gl_index_buffer.cpp
			gl/gl_index_buffer.hpp

			gl/gl_uniform_buffer.cpp
			gl/gl_uniform_buffer.hpp

			gl/gl_shader.cpp
			gl/gl_shader.hpp

			gl/gl_texture.cpp
			gl/gl_texture.hpp

			gl/gl_material.cpp
			gl/gl_material.hpp
	)
endif ()

if (${WITH_VULKAN_BACKEND})
	list(APPEND GPU_SRC
			vk/vk_gpu_context.hpp
			vk/vk_gpu_context.cpp
	)
endif ()

add_library(toast_gpu STATIC)

target_include_directories(toast_gpu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(toast_gpu PRIVATE ${GPU_SRC})

target_link_libraries(toast_gpu PUBLIC tst::toast_lib)
target_link_libraries(toast_gpu PUBLIC glfw)
target_link_libraries(toast_gpu PUBLIC stb)
target_link_libraries(toast_gpu PUBLIC assimp::assimp)


if (${WITH_OPENGL_BACKEND})
	target_link_libraries(toast_gpu PUBLIC openglhpp)

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

endif ()

add_library(tst::toast_gpu ALIAS toast_gpu)