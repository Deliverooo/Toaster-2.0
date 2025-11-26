# Compiler-specific settings for Toaster Engine

if(MSVC)
    # Visual Studio specific settings
    add_compile_options(
        /W4                 # Warning level 4
        /WX                 # Warnings as errors
        /utf-8              # UTF-8 source encoding
        /permissive-        # Disable non-conforming code
        /Zc:__cplusplus     # Correct __cplusplus macro
    )
    
    # Configuration-specific options
    add_compile_options("$<$<CONFIG:DEBUG>:/MDd>")
    add_compile_options("$<$<CONFIG:RELEASE>:/MD>")
    add_compile_options("$<$<CONFIG:RELEASE>:/O2>")
    
    # Enable parallel compilation
    add_compile_options(/MP)
    
    # Modern C++ features
    string(APPEND CMAKE_CXX_FLAGS " /Zc:preprocessor")
    string(APPEND CMAKE_C_FLAGS " /Zc:preprocessor")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang specific settings
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wno-unused-parameter
    )
    
    # Configuration-specific options
    add_compile_options("$<$<CONFIG:DEBUG>:-g>")
    add_compile_options("$<$<CONFIG:DEBUG>:-O0>")
    add_compile_options("$<$<CONFIG:RELEASE>:-O3>")
    add_compile_options("$<$<CONFIG:RELEASE>:-DNDEBUG>")
    
    # Additional GCC-specific warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-Wlogical-op)
    endif()
    
    # Additional Clang-specific warnings
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-c++98-compat)
    endif()
endif()

# Platform-specific settings
if(TST_PLATFORM_WINDOWS)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
elseif(TST_PLATFORM_LINUX)
    # Linux-specific settings
    find_package(PkgConfig REQUIRED)
endif()