set(SRC
		main.cpp
		client_application.cpp
		client_application.hpp
		client_layer.cpp
		client_layer.hpp
		imgui_layer.cpp
		imgui_layer.hpp
		editor_camera.cpp
		editor_camera.hpp
)

add_executable(toaster_client ${SRC})

target_link_libraries(toaster_client PRIVATE Toaster)
target_link_libraries(toaster_client PRIVATE imgui)