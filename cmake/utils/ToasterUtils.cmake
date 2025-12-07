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