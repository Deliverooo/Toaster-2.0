add_subdirectory(fmt)

add_subdirectory(assimp)

add_subdirectory(miniaudio)

add_subdirectory(stb)

add_subdirectory(entt)

add_subdirectory(nativefiledialog-extended)

add_subdirectory(yaml-cpp)
if (MSVC)
	# Something to do with stl base classes not being exported to the dll. Not important.
	target_compile_options(yaml-cpp PRIVATE /wd4251)
	target_compile_options(yaml-cpp PRIVATE /wd4275)
endif ()

add_subdirectory(glm)
target_compile_definitions(glm PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(glm-header-only INTERFACE GLM_ENABLE_EXPERIMENTAL)

if (${WITH_OPENGL_BACKEND})
	add_subdirectory(opengl-hpp)
endif ()

add_subdirectory(imgui)

add_subdirectory(ImGuizmo)

add_subdirectory(glfw)