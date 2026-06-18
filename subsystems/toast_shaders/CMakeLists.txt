set(SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

set(SHADER_SPV_OUTPUTS "")

set(SHADER_SOURCES_VULKAN
		${CMAKE_CURRENT_SOURCE_DIR}/depth_pre.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/geometry.vert.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/geometry.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/composite.vert.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/composite.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/skybox.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/quad.vert.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/quad.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/ssao.vert.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/ssao.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/fullscreen_quad.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/equirectangular_to_cube_map.comp.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/diffuse_irradiance_convolution.comp.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/ssao_blur.comp.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/ambient_occlusion.comp.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/quad_dynamic.pixel.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/mesh_geo.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/test_shader.mesh.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/test_shader.pixel.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/fullscreen_quad.mesh.glsl

)

set(SHADER_SOURCES_HLSL
		${CMAKE_CURRENT_SOURCE_DIR}/fullscreen_quad.vert.hlsl
		${CMAKE_CURRENT_SOURCE_DIR}/depth_pre.vert.hlsl
		${CMAKE_CURRENT_SOURCE_DIR}/mesh_dynamic.vert.hlsl
		${CMAKE_CURRENT_SOURCE_DIR}/quad_dynamic.vert.hlsl
		${CMAKE_CURRENT_SOURCE_DIR}/skybox.vert.hlsl
		${CMAKE_CURRENT_SOURCE_DIR}/mesh_geo.vert.hlsl
)


set(SHADER_HEADERS_TARGET_SRC "")

foreach (SHADER_FILE IN LISTS SHADER_SOURCES_VULKAN)

	cmake_path(GET SHADER_FILE FILENAME SHADER_FILENAME_WEXT)
	set(SHADER_FILE_OUT "${SHADER_OUTPUT_DIR}/${SHADER_FILENAME_WEXT}.spv")

	cmake_path(GET SHADER_FILE STEM SHADER_FILENAME)

	if (SHADER_FILE MATCHES "\\.vert\\.glsl$")

		set(SHADER_TYPE vert)
		set(SHADER_VARIABLE_NAME "g_vs_${SHADER_FILENAME}")
	elseif (SHADER_FILE MATCHES "\\.pixel\\.glsl$")

		set(SHADER_TYPE frag)
		set(SHADER_VARIABLE_NAME "g_ps_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.comp\\.glsl$")

		set(SHADER_TYPE comp)
		set(SHADER_VARIABLE_NAME "g_cs_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.mesh\\.glsl$")

		set(SHADER_TYPE mesh)
		set(SHADER_VARIABLE_NAME "g_ms_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.rgen\\.glsl$")

		set(SHADER_TYPE rgen)
		set(SHADER_VARIABLE_NAME "g_rgen_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.rint\\.glsl$")

		set(SHADER_TYPE rint)
		set(SHADER_VARIABLE_NAME "g_rgint_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.rahit\\.glsl$")

		set(SHADER_TYPE rahit)
		set(SHADER_VARIABLE_NAME "g_rahit_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.rchit\\.glsl$")

		set(SHADER_TYPE rchit)
		set(SHADER_VARIABLE_NAME "g_rchit_${SHADER_FILENAME}")

	elseif (SHADER_FILE MATCHES "\\.rcall\\.glsl$")

		set(SHADER_TYPE rcall)
		set(SHADER_VARIABLE_NAME "g_rcall_${SHADER_FILENAME}")
	endif ()

	add_custom_command(
			OUTPUT ${SHADER_FILE_OUT}
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} --target-env vulkan1.4 --invert-y -S ${SHADER_TYPE} -V "${SHADER_FILE}" -o "${SHADER_FILE_OUT}"
			DEPENDS ${SHADER_FILE}
			COMMENT "Compiling GLSL to ${SHADER_FILE_OUT}"
			VERBATIM
	)

	#	message(STATUS "Compiling GLSL to ${SHADER_FILE_OUT}")
	#
	#	set_source_files_properties(${SHADER_FILE_OUT} PROPERTIES GENERATED TRUE)
	#
	#	list(APPEND SHADER_HEADERS_TARGET_SRC ${SHADER_FILE_OUT})

	list(APPEND SHADER_SPV_OUTPUTS "${SHADER_FILE_OUT}")
endforeach ()

foreach (SHADER_FILE IN LISTS SHADER_SOURCES_HLSL)

	cmake_path(GET SHADER_FILE FILENAME SHADER_FILENAME_WEXT)
	set(SHADER_FILE_OUT "${SHADER_OUTPUT_DIR}/${SHADER_FILENAME_WEXT}.spv")

	cmake_path(GET SHADER_FILE STEM SHADER_FILENAME)

	if (SHADER_FILE MATCHES "\\.vert\\.hlsl$")
		set(SHADER_TYPE vs_6_6)
	elseif (SHADER_FILE MATCHES "\\.pixel\\.hlsl$")
		set(SHADER_TYPE ps_6_6)
	elseif (SHADER_FILE MATCHES "\\.comp\\.hlsl$")
		set(SHADER_TYPE cs_6_6)
	elseif (SHADER_FILE MATCHES "\\.rgen\\.hlsl$")
		set(SHADER_TYPE rgens_6_6)
	elseif (SHADER_FILE MATCHES "\\.rint\\.hlsl$")
		set(SHADER_TYPE rints_6_6)
	elseif (SHADER_FILE MATCHES "\\.rahit\\.hlsl$")
		set(SHADER_TYPE rahits_6_6)
	elseif (SHADER_FILE MATCHES "\\.rchit\\.hlsl$")
		set(SHADER_TYPE rchits_6_6)
	elseif (SHADER_FILE MATCHES "\\.rcall\\.hlsl$")
		set(SHADER_TYPE rcalls_6_6)
	endif ()

	add_custom_command(
			OUTPUT ${SHADER_FILE_OUT}
			COMMAND ${Vulkan_dxc_EXECUTABLE} -spirv -T ${SHADER_TYPE} -E main "${SHADER_FILE}" -Fo "${SHADER_FILE_OUT}" -fvk-use-dx-layout -fvk-invert-y  -fspv-use-legacy-buffer-matrix-order -fspv-target-env=vulkan1.3  -fspv-use-descriptor-heap -fspv-extension=SPV_EXT_descriptor_heap -fspv-extension=SPV_KHR_untyped_pointers -fspv-extension=SPV_KHR_physical_storage_buffer
			DEPENDS ${SHADER_FILE}
			COMMENT "Compiling HLSL to ${SHADER_FILE_OUT}"
			VERBATIM
	)

	#	message(STATUS "Compiling GLSL to ${SHADER_FILE_OUT}")
	#
	#	set_source_files_properties(${SHADER_FILE_OUT} PROPERTIES GENERATED TRUE)
	#
	#	list(APPEND SHADER_HEADERS_TARGET_SRC ${SHADER_FILE_OUT})

	list(APPEND SHADER_SPV_OUTPUTS "${SHADER_FILE_OUT}")
endforeach ()


add_custom_target(shader_headers_target DEPENDS "${SHADER_SPV_OUTPUTS}")

add_library(ToastShaders INTERFACE)

target_include_directories(ToastShaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_dependencies(ToastShaders shader_headers_target)

add_library(Toaster::ToastShaders ALIAS ToastShaders)


include(GNUInstallDirs)

install(
		FILES ${SHADER_SPV_OUTPUTS}
		DESTINATION "${CMAKE_INSTALL_BINDIR}/shaders"
)

install(
		TARGETS ToastShaders
		EXPORT ToastShadersTargets
)