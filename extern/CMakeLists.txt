set(ARG ON CACHE BOOL "" FORCE)
add_subdirectory(argparse)

set(FMT_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(fmt)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(assimp)

add_subdirectory(stb)

set(ENTT_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(entt)

set(NFD_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(nativefiledialog-extended)

set(YAML_CPP_INSTALL ON CACHE BOOL "" FORCE)
add_subdirectory(yaml-cpp)
if (MSVC)
	# Something to do with stl base classes not being exported to the dll. Not important.
	target_compile_options(yaml-cpp PUBLIC /wd4251)
	target_compile_options(yaml-cpp PUBLIC /wd4275)
endif ()

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_SHARED_LIBRARY ON CACHE BOOL "" FORCE)

set(GLFW_INSTALL ON CACHE BOOL "" FORCE)

add_subdirectory(glfw)

add_subdirectory(meshoptimizer)