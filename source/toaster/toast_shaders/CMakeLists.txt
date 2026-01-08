set(SHADER_SOURCES_VULKAN
		${CMAKE_CURRENT_SOURCE_DIR}/test.vert.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/mesh.vert.glsl

		${CMAKE_CURRENT_SOURCE_DIR}/test.pixel.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/mesh.pixel.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/mandlebrot.pixel.glsl
		${CMAKE_CURRENT_SOURCE_DIR}/planet.pixel.glsl
)

set(SHADER_HEADERS_TARGET_SRC "")

foreach (SHADER_FILE IN LISTS SHADER_SOURCES_VULKAN)

	cmake_path(GET SHADER_FILE FILENAME SHADER_FILENAME_WEXT)
	set(HEADER_OUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILENAME_WEXT}.spv.inl")

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
			OUTPUT ${HEADER_OUT}
			COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} --target-env vulkan1.4 -S ${SHADER_TYPE} -V --vn ${SHADER_VARIABLE_NAME} "${SHADER_FILE}" -o "${HEADER_OUT}"
			DEPENDS ${SHADER_FILE}
			COMMENT "Compiling GLSL to ${HEADER_OUT}"
			VERBATIM
	)

	message(STATUS "Compiling GLSL to ${HEADER_OUT}")

	set_source_files_properties(${HEADER_OUT} PROPERTIES GENERATED TRUE)

	list(APPEND SHADER_HEADERS_TARGET_SRC ${HEADER_OUT})
endforeach ()


add_custom_target(shader_headers_target DEPENDS "${SHADER_HEADERS_TARGET_SRC}")

add_library(toast_shaders INTERFACE)

target_include_directories(toast_shaders INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")

add_dependencies(toast_shaders shader_headers_target)

add_library(tst::toast_shaders ALIAS toast_shaders)