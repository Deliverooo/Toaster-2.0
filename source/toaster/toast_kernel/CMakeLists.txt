set(SRC
		application.cpp
		application.hpp

		input.cpp
		input.hpp

		window.cpp
		window.hpp

		camera.cpp
		camera.hpp

		mesh.cpp
		mesh.hpp
)


add_library(toast_kernel STATIC)

target_sources(toast_kernel PRIVATE ${SRC})
target_include_directories(toast_kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(toast_kernel PUBLIC tst::toast_lib)
target_link_libraries(toast_kernel PUBLIC glfw)
target_link_libraries(toast_kernel PUBLIC tst::toast_shaders)
target_link_libraries(toast_kernel PUBLIC tst::toast_gpu)
target_link_libraries(toast_kernel PUBLIC assimp::assimp)
target_link_libraries(toast_kernel PUBLIC stb)


if (${Vulkan_FOUND})
	message(NOTICE "Found Vulkan and glslandValidator! :)")
	target_link_libraries(toast_kernel PUBLIC Vulkan::Vulkan)

else ()
	message(FATAL_ERROR "Vulkan enabled as a graphics backend, but cannot be found!")
endif ()

add_library(tst::toast_kernel ALIAS toast_kernel)
