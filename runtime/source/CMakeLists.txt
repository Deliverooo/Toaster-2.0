set(SRC
		main.cpp
		runtime_application.cpp
		runtime_application.hpp
		runtime_layer.cpp
		runtime_layer.hpp
		fp_camera.cpp
		fp_camera.hpp
		toaster_runtime.rc
)

add_executable(toaster_runtime ${SRC})

target_link_libraries(toaster_runtime PRIVATE Toaster)
target_link_libraries(toaster_runtime PRIVATE imgui)
target_link_libraries(toaster_runtime PRIVATE argparse)