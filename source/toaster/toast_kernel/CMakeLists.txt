set(SRC
		application.cpp
		application.hpp

		input.cpp
		input.hpp

		window.cpp
		window.hpp

		ortho_camera_controller.cpp
		ortho_camera_controller.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})

#add_library(toast_kernel STATIC)
#
#target_sources(toast_kernel PRIVATE ${SRC})
#target_include_directories(toast_kernel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#
#target_link_libraries(toast_kernel PUBLIC ${PLATFORM_LINKLIBS})
#
#add_library(tst::toast_kernel ALIAS toast_kernel)