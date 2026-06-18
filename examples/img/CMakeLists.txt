cmake_minimum_required(VERSION 3.30)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)

if (MSVC)
	add_compile_options(/MP)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
# Prevent in-source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif ()

# Project definition
project("img")

find_package(Toaster REQUIRED)

# Select C++23 as the standard for C++ projects.
# If C++23 is not available, downgrading to an earlier standard is NOT OK.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler specific language extensions.
set(CMAKE_CXX_EXTENSIONS OFF)

# Exe setup
add_executable("img")

# Make sure that no command line shows up in release mode
set_target_properties("img" PROPERTIES
		WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>"
)

set(SOURCE_FILES
		src/image_viewer_camera.cpp
		src/new_layer.cpp
		src/main.cpp
)

target_sources("img" PRIVATE ${SOURCE_FILES})

target_include_directories("img" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries("img" PRIVATE Toaster::ToastKernel)
