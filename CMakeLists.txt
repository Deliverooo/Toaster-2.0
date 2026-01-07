cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Prevent in-source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not allowed. Please create a separate build directory.")
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/platform")


# Project definition
project(Toaster)

if (DEFINED WITH_VULKAN_BACKEND)
	message(NOTICE "WARNING: WITH_VULKAN_BACKEND was already defined as: ${WITH_VULKAN_BACKEND}")
endif ()
option(WITH_VULKAN_BACKEND "Enable Vulkan as graphics backend" ON)

mark_as_advanced(WITH_VULKAN_BACKEND)
message(STATUS "DEBUG: WITH_VULKAN_BACKEND is set to: '${WITH_VULKAN_BACKEND}'")


if (WITH_VULKAN_BACKEND)
	find_package(Vulkan REQUIRED COMPONENTS glslangValidator shaderc_combined SPIRV-Tools)

	if (NOT ${Vulkan_FOUND} OR NOT ${Vulkan_glslangValidator_FOUND})
		message(FATAL_ERROR "Vulkan enabled as a graphics backend, but cannot be found!")

	else ()
		message(NOTICE "Found Vulkan and glslandValidator! :)")
	endif ()

endif ()


# Dx11 / Dx12
if (WIN32)
	option(WITH_DX11_BACKEND "Enable Dx11 as graphics backend" OFF)
	option(WITH_DX12_BACKEND "Enable Dx12 as graphics backend" OFF)
	mark_as_advanced(WITH_DX11_BACKEND)
	mark_as_advanced(WITH_DX12_BACKEND)
else ()
	set(WITH_DX11_BACKEND OFF)
	set(WITH_DX12_BACKEND OFF)
endif ()

# C/C++ flags
set(PLATFORM_CFLAGS)

# these are added to later on.
set(C_WARNINGS)
set(CXX_WARNINGS)


set(PLATFORM_LINKLIBS "")


# ----------------------------------------------------------------------------
# Main Platform Checks
#
# - UNIX
# - WIN32
# - APPLE

if (UNIX AND NOT APPLE)
	include(platform_unix)
elseif (WIN32)
	include(platform_win32)
elseif (APPLE)
	include(platform_apple)
endif ()

if (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
	# Most MSVC warnings are C & C++.
	set(_WARNINGS
			# warning level:
			"/W3"
			"/w34062"  # switch statement contains 'default' but no 'case' labels
			"/w34100"  # 'identifier' : unreferenced formal parameter
			"/w34115"  # 'type' : named type definition in parentheses
			"/w34189"  # local variable is initialized but not referenced
			# see https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5038?view=vs-2017
			"/w35038"  # order of initialization in c++ constructors
			# disable:
			"/wd4018"  # signed/unsigned mismatch
			"/wd4146"  # unary minus operator applied to unsigned type, result still unsigned
			"/wd4065"  # switch statement contains 'default' but no 'case' labels
			"/wd4127"  # conditional expression is constant
			"/wd4181"  # qualifier applied to reference type; ignored
			"/wd4200"  # zero-sized array in struct/union
			"/wd4244"  # conversion from 'type1' to 'type2', possible loss of data
			"/wd4267"  # conversion from 'size_t' to 'type', possible loss of data
			"/wd4305"  # truncation from 'type1' to 'type2'
			"/wd4800"  # forcing value to bool 'true' or 'false'
			"/wd4828"  # The file contains a character that is illegal
			"/wd4996"  # identifier was declared deprecated
			"/wd4661"  # no suitable definition provided for explicit template instantiation request
			"/wd4848"  # 'no_unique_address' is a vendor extension in C++17
			# errors:
			"/we4013"  # 'function' undefined; assuming extern returning int
			"/we4133"  # incompatible pointer types
			"/we4431"  # missing type specifier - int assumed
			"/we4033"  # 'function' must return a value
	)
	string(REPLACE ";" " " _WARNINGS "${_WARNINGS}")
	set(C_WARNINGS "${_WARNINGS}")
	set(CXX_WARNINGS "${_WARNINGS}")
	unset(_WARNINGS)
endif ()

# Select C++23 as the standard for C++ projects.
set(CMAKE_CXX_STANDARD 23)
# If C++23 is not available, downgrading to an earlier standard is NOT OK.
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler specific language extensions.
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_FLAGS "${C_WARNINGS} ${CMAKE_C_FLAGS} ${PLATFORM_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CXX_WARNINGS} ${CMAKE_CXX_FLAGS} ${PLATFORM_CFLAGS}")

add_compile_definitions(TST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/release/")


add_subdirectory(extern)
add_subdirectory(source)

add_subdirectory(source/launcher)