set(SRC
		main.cpp
)

add_executable(tstb ${SRC})

target_link_libraries(tstb PRIVATE Toaster)
target_link_libraries(tstb PRIVATE argparse)