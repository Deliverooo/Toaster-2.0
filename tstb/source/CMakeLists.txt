set(SRC
		main.cpp

		tstb.cpp
		tstb.hpp

		toaster_build.rc
)

add_executable(tstb ${SRC})

target_link_libraries(tstb PRIVATE Toaster)
target_link_libraries(tstb PRIVATE argparse)
target_link_libraries(tstb PRIVATE assimp::assimp)