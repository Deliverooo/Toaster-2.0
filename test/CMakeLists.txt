set(SRC
		main.cpp
)

add_executable(toaster_test WIN32 ${SRC} )

target_link_libraries(toaster_test PRIVATE
		Toaster::ToastOS
		Toaster::ToastLib
		Toaster::ToastGPU
)