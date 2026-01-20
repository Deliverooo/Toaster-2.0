add_subdirectory(assimp)

add_subdirectory(fmt)

add_subdirectory(stb)

add_subdirectory(cpp-httplib)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)


if (${WITH_OPENGL_BACKEND})
	add_subdirectory(openglhpp)
endif ()

if (${WITH_VULKAN_BACKEND})
	set(NVRHI_WITH_VULKAN "${WITH_VULKAN_BACKEND}" CACHE BOOL "" FORCE)
	add_subdirectory(nvrhi)

endif ()


add_subdirectory(imgui)


add_subdirectory(glfw)