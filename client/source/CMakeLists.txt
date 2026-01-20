set(LIB

		PRIVATE tst::toast_kernel
)

set(SRC
		main.cpp
		client_application.cpp
		client_application.hpp
		client_layer.cpp
		client_layer.hpp
)

add_executable(toaster_client ${SRC})

target_link_libraries(toaster_client PRIVATE ${LIB})

