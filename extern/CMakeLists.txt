add_subdirectory(argparse)

set(FMT_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(fmt)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp)

add_subdirectory(miniaudio)

add_subdirectory(stb)

add_subdirectory(entt)

set(NFD_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(nativefiledialog-extended)

add_subdirectory(yaml-cpp)
if (MSVC)
	# Something to do with stl base classes not being exported to the dll. Not important.
	target_compile_options(yaml-cpp PUBLIC /wd4251)
	target_compile_options(yaml-cpp PUBLIC /wd4275)
endif ()


#target_compile_definitions(glm PUBLIC GLM_ENABLE_EXPERIMENTAL)
#target_compile_definitions(glm-header-only INTERFACE GLM_ENABLE_EXPERIMENTAL)

if (${WITH_OPENGL_BACKEND})
	add_subdirectory(opengl-hpp)
endif ()

add_subdirectory(imgui)

add_subdirectory(ImGuizmo)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_SHARED_LIBRARY ON CACHE BOOL "" FORCE)

add_subdirectory(glfw)