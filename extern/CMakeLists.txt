# External dependencies configuration
message(STATUS "Configuring external dependencies...")

# Dependencies are now handled by ToasterDependencies.cmake
# This file can contain additional external library configurations
# or overrides if needed

# Set folder for external targets in IDE

add_subdirectory(glm)

add_subdirectory(GLFW)

add_subdirectory(glad)

add_subdirectory(spdlog)

add_subdirectory(fmt)

add_subdirectory(nvrhi)

message(STATUS "External dependencies configured")