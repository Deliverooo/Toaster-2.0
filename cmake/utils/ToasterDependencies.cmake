# External dependency management for Toaster Engine

include(FetchContent)

# Configure external libraries for shared builds
if (NOT TST_STATIC_LIBS)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    message(STATUS "Configuring dependencies for shared library build")
else ()
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    message(STATUS "Configuring dependencies for static library build")
endif ()