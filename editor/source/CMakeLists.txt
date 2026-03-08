set(SRC
		main.cpp
		editor_application.cpp
		editor_application.hpp
		editor_layer.cpp
		editor_layer.hpp
		imgui_layer.cpp
		imgui_layer.hpp
)

add_executable(toaster_editor ${SRC})

target_link_libraries(toaster_editor PRIVATE Toaster)

target_link_libraries(toaster_editor PRIVATE imgui)

