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
target_link_libraries(Toaster PUBLIC EnTT::EnTT)
target_link_libraries(Toaster PRIVATE yaml-cpp::yaml-cpp)

target_link_libraries(Toaster PUBLIC dwmapi) # For window colour

set_target_properties(Toaster PROPERTIES COMPILE_WARNING_AS_ERROR ON)

set(DOTNET_VERSION "9.0.14")
set(DOTNET_ROOT "C:/Program Files/dotnet")

set(NETHOST_PATH "${DOTNET_ROOT}/packs/Microsoft.NETCore.App.Host.win-x64/${DOTNET_VERSION}/runtimes/win-x64/native")

target_include_directories(Toaster PUBLIC ${NETHOST_PATH})

find_library(NETHOST_LIB nethost PATHS ${NETHOST_PATH} NO_DEFAULT_PATH)
target_link_libraries(Toaster PUBLIC ${NETHOST_LIB})

if (WIN32)
	add_custom_command(TARGET Toaster POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${NETHOST_PATH}/nethost.dll"
			$<TARGET_FILE_DIR:Toaster>)
endif ()
