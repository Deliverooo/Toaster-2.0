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

target_link_libraries(Toaster PUBLIC dwmapi) # For window colour

set_target_properties(Toaster PROPERTIES COMPILE_WARNING_AS_ERROR ON)

set(DOTNET_VERSION "9.0.14")
set(DOTNET_ROOT "C:/Program Files/dotnet")

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
