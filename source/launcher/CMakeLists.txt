set(INCLUDE_DIRS "")

set(LIB

		PRIVATE tst::toast_kernel
)

if (WITH_OPENGL_BACKEND)
	add_definitions(-DWITH_OPENGL_BACKEND)
endif ()

if (WITH_VULKAN_BACKEND)
	add_definitions(-DWITH_VULKAN_BACKEND)
endif ()


if (WIN32)
	add_definitions(-DNOMINMAX)
endif ()


set(SOURCE_FILES "main.cpp")

add_executable(toaster ${SOURCE_FILES})

target_link_libraries(toaster PRIVATE ${LIB})

