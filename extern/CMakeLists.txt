add_subdirectory(fmt)

add_subdirectory(assimp)

add_subdirectory(miniaudio)

add_subdirectory(stb)

add_subdirectory(entt)

add_subdirectory(nativefiledialog-extended)

#add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL)
#set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)
target_compile_definitions(glm PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(glm-header-only INTERFACE GLM_ENABLE_EXPERIMENTAL)

if (${WITH_OPENGL_BACKEND})
	add_subdirectory(openglhpp)
endif ()

add_subdirectory(imgui)

add_subdirectory(glfw)