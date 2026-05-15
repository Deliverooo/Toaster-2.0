set(GPU_SRC
		toast_gpu.hpp
		image.hpp
		resource.hpp
		buffer_layout.hpp
)

if (${WITH_VULKAN_BACKEND})
	list(APPEND GPU_SRC
			vk/vk_instance.hpp
			vk/vk_instance.cpp

			vk/vk_physical_device.hpp
			vk/vk_physical_device.cpp

			vk/vk_logical_device.hpp
			vk/vk_logical_device.cpp

			vk/vk_command_buffer.hpp
			vk/vk_command_buffer.cpp

			vk/vk_swapchain.cpp
			vk/vk_swapchain.hpp

			vk/vk_pipeline.cpp
			vk/vk_pipeline.hpp

			vk/vk_vertex_buffer.cpp
			vk/vk_vertex_buffer.hpp

			vk/vk_index_buffer.cpp
			vk/vk_index_buffer.hpp

			vk/vk_uniform_buffer.cpp
			vk/vk_uniform_buffer.hpp

			vk/vk_shader.cpp
			vk/vk_shader.hpp

			vk/vk_shader_compiler.cpp
			vk/vk_shader_compiler.hpp

			vk/vk_shader_resources.hpp

			vk/vk_common.cpp
			vk/vk_common.hpp

			vk/vk_image.cpp
			vk/vk_image.hpp

			vk/vk_texture.cpp
			vk/vk_texture.hpp

			vk/vk_render_pass.cpp
			vk/vk_render_pass.hpp

			vk/vk_descriptor_set_manager.cpp
			vk/vk_descriptor_set_manager.hpp

			vk/vk_render_attachment.cpp
			vk/vk_render_attachment.hpp

			vk/vk_compute_pass.cpp
			vk/vk_compute_pass.hpp

			vk/vk_compute_pipeline.cpp
			vk/vk_compute_pipeline.hpp

			vk/vk_storage_buffer.cpp
			vk/vk_storage_buffer.hpp
	)
endif ()

add_library(toast_gpu SHARED)

target_include_directories(toast_gpu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_sources(toast_gpu PRIVATE ${GPU_SRC})

target_link_libraries(toast_gpu PUBLIC tst::toast_lib)
target_link_libraries(toast_gpu PUBLIC stb)
target_link_libraries(toast_gpu PUBLIC assimp::assimp)

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

add_compile_definitions(toast_gpu PUBLIC TST_GPU_BUILD_DLL)
add_compile_definitions(toast_gpu PRIVATE TST_GPU_DLL_EXPORT)

