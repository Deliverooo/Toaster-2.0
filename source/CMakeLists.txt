add_library(Toaster SHARED)
target_include_directories(Toaster PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(toaster)

add_compile_definitions(Toaster PUBLIC TST_BUILD_DLL)
add_compile_definitions(Toaster PRIVATE TST_DLL_EXPORT)

target_link_libraries(Toaster PUBLIC tst::toast_lib)
target_link_libraries(Toaster PUBLIC tst::toast_ui)
target_link_libraries(Toaster PUBLIC tst::toast_gpu)

target_link_libraries(Toaster PRIVATE assimp::assimp)
target_link_libraries(Toaster PRIVATE stb)
target_link_libraries(Toaster PUBLIC glfw)
target_compile_definitions(Toaster PUBLIC GLFW_DLL)

target_link_libraries(Toaster PUBLIC EnTT::EnTT)
target_link_libraries(Toaster PRIVATE yaml-cpp::yaml-cpp)

target_link_libraries(Toaster PUBLIC argparse)

target_link_libraries(Toaster PUBLIC dwmapi) # For window colour


#set(FFX_SDK_DIR "C:/dev/FidelityFX-SDK")
#message(STATUS "Fidelity Fx SDK Path: ${FFX_SDK_DIR}")
#
#set(FFX_INCLUDE_DIR  "${FFX_SDK_DIR}/sdk/include")
#set(FFX_BINARY_DIR   "${FFX_SDK_DIR}/sdk/bin/ffx_sdk")
#
#add_library(FidelityFX::backend_vk SHARED IMPORTED GLOBAL)
#set_target_properties(FidelityFX::backend_vk PROPERTIES
#		IMPORTED_IMPLIB "${FFX_BINARY_DIR}/ffx_backend_vk_x64.lib"
#		IMPORTED_LOCATION "${FFX_BINARY_DIR}/ffx_backend_vk_x64.dll"
#)
#
#add_library(FidelityFX::cacao SHARED IMPORTED GLOBAL)
#set_target_properties(FidelityFX::cacao PROPERTIES
#		IMPORTED_IMPLIB "${FFX_BINARY_DIR}/ffx_cacao_x64.lib"
#		IMPORTED_LOCATION "${FFX_BINARY_DIR}/ffx_cacao_x64.dll"
#)
#
#target_include_directories(Toaster PUBLIC ${FFX_INCLUDE_DIR})
#target_link_libraries(Toaster PUBLIC FidelityFX::backend_vk)
#target_link_libraries(Toaster PUBLIC FidelityFX::cacao)
#if(WIN32)
#	add_custom_command(TARGET Toaster POST_BUILD
#			COMMAND ${CMAKE_COMMAND} -E copy_if_different
#			"$<TARGET_FILE:FidelityFX::backend_vk>"
#			"$<TARGET_FILE:FidelityFX::cacao>"
#			"$<TARGET_FILE_DIR:Toaster>"
#			COMMENT "Deploying FidelityFX runtime DLL modules to output directory..."
#	)
#endif()


add_dependencies(Toaster toast_shaders)

set_target_properties(Toaster PROPERTIES COMPILE_WARNING_AS_ERROR ON)

set(MONO_PATH "C:/Program Files/Mono")
target_include_directories(Toaster PUBLIC "${MONO_PATH}/include/mono-2.0")
find_library(MONO_LIB NAMES mono-2.0-sgen PATHS "${MONO_PATH}/lib")

set(MONO_DLL_PATH "${MONO_PATH}/bin/mono-2.0-sgen.dll")
target_link_libraries(Toaster PUBLIC ${MONO_LIB})
if (WIN32)
	add_custom_command(TARGET Toaster POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${MONO_DLL_PATH}"
			$<TARGET_FILE_DIR:Toaster>)
endif ()
