cmake_minimum_required(VERSION 3.10)

# Prevent in-source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif ()

# Project definition
project(Toaster)
find_package(Vulkan)

if (Vulkan_FOUND)
	set(TST_ENABLE_VULKAN ON)
	message(STATUS "Vulkan SDK found")
else ()
	set(TST_ENABLE_VULKAN OFF)
	message(WARNING "Vulkan SDK not found - disabling Vulkan support")
endif ()


# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/utils")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Configuration options
option(TST_STATIC_LIBS "Build as static libraries" ON)

option(TST_ENABLE_OPENGL "Enable OpenGL support" ON)
#option(TST_ENABLE_VULKAN "Enable Vulkan support (requires Vulkan SDK)" ON)


# Platform detection
if (WIN32)
	set(TST_PLATFORM_WINDOWS TRUE)
	add_compile_definitions(TST_PLATFORM_WINDOWS)
	message(STATUS "Building for Windows platform")
elseif (APPLE)
	set(TST_PLATFORM_MACOS TRUE)
	add_compile_definitions(TST_PLATFORM_MACOS)
	message(STATUS "Building for macOS platform")
elseif (UNIX)
	set(TST_PLATFORM_LINUX TRUE)
	add_compile_definitions(TST_PLATFORM_LINUX)
	message(STATUS "Building for Linux platform")
endif ()

include(ToasterUtils)


# Add subdirectories
add_subdirectory(extern)
add_subdirectory(engine)

# Print configuration summary
message(STATUS "=== Toaster Engine Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Library type: ${TST_STATIC_LIBS}")
message(STATUS "OpenGL enabled: ${TST_ENABLE_OPENGL}")
message(STATUS "Vulkan enabled: ${TST_ENABLE_VULKAN}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "=====================================")

# Toaster Editor
add_executable(ToasterEditor)
add_subdirectory(tools/editor)

target_link_libraries(ToasterEditor PRIVATE
		ToasterEngine)

toaster_set_target_properties(ToasterEditor)
toaster_set_warnings(ToasterEditor)


# Sandbox
add_executable(Sandbox)
add_subdirectory(tools/sandbox)

target_link_libraries(Sandbox PRIVATE
		ToasterEngine)

toaster_set_target_properties(Sandbox)
toaster_set_warnings(Sandbox)

