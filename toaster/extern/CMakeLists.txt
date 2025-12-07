message(STATUS "Configuring external dependencies...")

add_subdirectory(glm)


option(GLFW_BUILD_EXAMPLES "" OFF)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
add_subdirectory(glfw)

add_subdirectory(glad)

add_subdirectory(spdlog)

add_subdirectory(fmt)


set(NVRHI_WITH_DX11 "${TST_ENABLE_DX11}" CACHE BOOL "" FORCE)
set(NVRHI_WITH_DX12 "${TST_ENABLE_DX12}" CACHE BOOL "" FORCE)
set(NVRHI_WITH_VULKAN "${TST_ENABLE_VULKAN}" CACHE BOOL "" FORCE)

add_subdirectory(nvrhi)

add_subdirectory(yaml-cpp)

add_subdirectory(imgui)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(shaderc)

set(SPIRV_CROSS_CLI OFF)
add_subdirectory(SPIRV-Cross)

message(STATUS "External dependencies configured")