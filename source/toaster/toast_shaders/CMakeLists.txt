set(SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

if (${WITH_VULKAN_BACKEND})
	set(SHADER_SOURCES_VULKAN
			${CMAKE_CURRENT_SOURCE_DIR}/depth-pre.vert.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/depth-pre.pixel.glsl

			${CMAKE_CURRENT_SOURCE_DIR}/geometry.vert.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/geometry.pixel.glsl

			${CMAKE_CURRENT_SOURCE_DIR}/composite.vert.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/composite.pixel.glsl

			${CMAKE_CURRENT_SOURCE_DIR}/skybox.vert.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/skybox.pixel.glsl

			${CMAKE_CURRENT_SOURCE_DIR}/quad.vert.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/quad.pixel.glsl

			${CMAKE_CURRENT_SOURCE_DIR}/anti-aliasing.vert.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/anti-aliasing.pixel.glsl

			${CMAKE_CURRENT_SOURCE_DIR}/test.comp.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/light_culling.comp.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/equirectangular_to_cubemap.comp.glsl
			${CMAKE_CURRENT_SOURCE_DIR}/ambient_occlusion.comp.glsl
	)
endif ()

set(SHADER_HEADERS_TARGET_SRC "")

if (${WITH_VULKAN_BACKEND})
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

		message(STATUS "Compiling GLSL to ${SHADER_FILE_OUT}")

		set_source_files_properties(${SHADER_FILE_OUT} PROPERTIES GENERATED TRUE)

		list(APPEND SHADER_HEADERS_TARGET_SRC ${SHADER_FILE_OUT})
	endforeach ()

endif ()

add_custom_target(shader_headers_target DEPENDS "${SHADER_HEADERS_TARGET_SRC}")

add_library(toast_shaders INTERFACE)

target_include_directories(toast_shaders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#set(XE_GTAO_SOURCES
#		${CMAKE_CURRENT_SOURCE_DIR}/XeGTAO/vaNoise.hlsl
#		${CMAKE_CURRENT_SOURCE_DIR}/XeGTAO/vaPostProcessShared.h
#		${CMAKE_CURRENT_SOURCE_DIR}/XeGTAO/vaShared.hlsl
#		${CMAKE_CURRENT_SOURCE_DIR}/XeGTAO/vaSharedTypes.h
#		${CMAKE_CURRENT_SOURCE_DIR}/XeGTAO/XeGTAO.h
#		${CMAKE_CURRENT_SOURCE_DIR}/XeGTAO/XeGTAO.hlsli
#)
#target_sources(toast_shaders INTERFACE ${XE_GTAO_SOURCES})

add_dependencies(toast_shaders shader_headers_target)

add_library(tst::toast_shaders ALIAS toast_shaders)