# Function to set warning levels
function(toaster_set_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
                 /utf-8 /permissive-
        )
    else ()
        target_compile_options(${target} PRIVATE
                -Wextra -Wpedantic -Werror
                -Wno-unused-parameter
        )
    endif ()
endfunction()

# Function to set common target properties with dependency awareness
function(toaster_set_target_properties target)
    set_target_properties(${target} PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS ON
            POSITION_INDEPENDENT_CODE ON
    )
endfunction()

# Enhanced module creation with better dependency handling
function(toaster_add_engine_module module_name)
    set(options HEADER_ONLY SHARED STATIC)
    set(oneValueArgs FOLDER NAMESPACE VERSION)
    set(multiValueArgs
            SOURCES
            HEADERS
            PUBLIC_DEPENDENCIES
            PRIVATE_DEPENDENCIES
            SYSTEM_DEPENDENCIES
            COMPILE_DEFINITIONS
            COMPILE_FEATURES
            INCLUDE_DIRECTORIES
    )
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Determine library type
    if (ARG_HEADER_ONLY)
        add_library(${module_name} INTERFACE)
        set(visibility INTERFACE)
        set(is_interface TRUE)
    else ()
        # Respect global setting or explicit override
        if (ARG_SHARED)
            add_library(${module_name} SHARED ${ARG_SOURCES} ${ARG_HEADERS})
        elseif (ARG_STATIC)
            add_library(${module_name} STATIC ${ARG_SOURCES} ${ARG_HEADERS})
        else ()
            # Use global setting
            if (TST_STATIC_LIBS)
                add_library(${module_name} STATIC ${ARG_SOURCES} ${ARG_HEADERS})
            else ()
                add_library(${module_name} SHARED ${ARG_SOURCES} ${ARG_HEADERS})
            endif ()
        endif ()
        set(visibility PUBLIC)
        set(is_interface FALSE)
    endif ()

    # Set include directories
    target_include_directories(${module_name} ${visibility}
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    # Add additional include directories
    if (ARG_INCLUDE_DIRECTORIES)
        target_include_directories(${module_name} ${visibility} ${ARG_INCLUDE_DIRECTORIES})
    endif ()

    # Add private source directory for non-header-only libraries
    if (NOT is_interface)
        target_include_directories(${module_name} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        toaster_set_target_properties(${module_name})
        toaster_set_warnings(${module_name})
    endif ()

    # Link dependencies with proper visibility
    if (ARG_PUBLIC_DEPENDENCIES)
        target_link_libraries(${module_name} ${visibility} ${ARG_PUBLIC_DEPENDENCIES})
    endif ()

    if (ARG_PRIVATE_DEPENDENCIES AND NOT is_interface)
        target_link_libraries(${module_name} PRIVATE ${ARG_PRIVATE_DEPENDENCIES})
    endif ()

    # Link system dependencies
    if (ARG_SYSTEM_DEPENDENCIES AND NOT is_interface)
        target_link_libraries(${module_name} PRIVATE ${ARG_SYSTEM_DEPENDENCIES})
    endif ()

    # Set compile definitions
    if (ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${module_name} ${visibility} ${ARG_COMPILE_DEFINITIONS})
    endif ()

    # Set compile features
    if (ARG_COMPILE_FEATURES)
        target_compile_features(${module_name} ${visibility} ${ARG_COMPILE_FEATURES})
    else ()
        target_compile_features(${module_name} ${visibility} cxx_std_23)
    endif ()

    # Set version if provided
    if (ARG_VERSION AND NOT is_interface)
        set_target_properties(${module_name} PROPERTIES VERSION ${ARG_VERSION})
    endif ()

    # Set IDE folder
    if (ARG_FOLDER)
        set_target_properties(${module_name} PROPERTIES FOLDER ${ARG_FOLDER})
    endif ()

    # Create alias with proper namespace
    set(namespace ${ARG_NAMESPACE})
    if (NOT namespace)
        set(namespace "Toaster")
    endif ()
    add_library(${namespace}::${module_name} ALIAS ${module_name})

    message(STATUS "Created module: ${namespace}::${module_name}")
endfunction()

# Function to copy shared libraries to output directory
function(toaster_copy_shared_libraries target)
    if (NOT TST_STATIC_LIBS)
        # Copy GLFW DLL on Windows
        if (TST_PLATFORM_WINDOWS AND TARGET glfw)
            add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:glfw>
                    $<TARGET_FILE_DIR:${target}>
                    COMMENT "Copying GLFW DLL"
            )
        endif ()

        # Similar for other dependencies...
        # Add more copy commands as needed
    endif ()
endfunction()

# Function to setup installation for shared libraries
function(toaster_install_target target)
    if (NOT TST_STATIC_LIBS)
        install(TARGETS ${target}
                EXPORT ToasterTargets
                LIBRARY DESTINATION lib
                ARCHIVE DESTINATION lib
                RUNTIME DESTINATION bin
                INCLUDES DESTINATION include
        )

        # Install headers for libraries
        get_target_property(TARGET_TYPE ${target} TYPE)
        if (TARGET_TYPE MATCHES "LIBRARY")
            if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
                install(DIRECTORY include/
                        DESTINATION include
                        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
                )
            endif ()
        endif ()
    endif ()
endfunction()