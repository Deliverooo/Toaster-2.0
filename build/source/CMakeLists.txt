set(SRC
		main.cpp
		toaster_build.rc
)

add_executable(tstb ${SRC})

target_link_libraries(tstb PRIVATE Toaster)
target_link_libraries(tstb PRIVATE argparse)