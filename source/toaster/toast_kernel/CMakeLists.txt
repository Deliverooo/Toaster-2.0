set(SRC
		application.cpp
		application.hpp

		input.cpp
		input.hpp

		window.cpp
		window.hpp

		camera.hpp

		renderer.hpp
		renderer.cpp

		ortho_camera.hpp
		ortho_camera.cpp

		render_command.cpp
		render_command.hpp

		events/event.hpp
		events/key_event.hpp
		events/mouse_event.hpp
		events/window_event.hpp

)


add_library(toast_kernel STATIC)

target_sources(toast_kernel PRIVATE ${SRC})
target_include_directories(toast_kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(toast_kernel PUBLIC tst::toast_lib)
target_link_libraries(toast_kernel PUBLIC tst::toast_gpu)
target_link_libraries(toast_kernel PUBLIC tst::toast_shaders)

target_link_libraries(toast_kernel PUBLIC glfw)

target_link_libraries(toast_kernel PUBLIC ${PLATFORM_LINKLIBS})

add_library(tst::toast_kernel ALIAS toast_kernel)