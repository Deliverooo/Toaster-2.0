set(SRC
		main.cpp
		launcher_application.cpp
		launcher_application.hpp
		launcher_layer.cpp
		launcher_layer.hpp
)

add_executable(toaster_launcher ${SRC})

target_link_libraries(toaster_launcher PRIVATE Toaster)
target_link_libraries(toaster_launcher PRIVATE argparse)