set(SRC
		logging.cpp
		logging.hpp

		toast_assert.cpp
		toast_assert.h

		ptr.hpp

		camera.hpp

		io/filesystem.cpp
		io/filesystem.hpp

		io/serializable.hpp

		io/stream_reader.hpp
		io/stream_writer.hpp

		io/file_stream.cpp
		io/file_stream.hpp

		math/math_vector.cpp
		math/math_vector.hpp
		math/math_constants.hpp
		math/math_matrix.hpp
		math/colours.hpp

		util_defines.hpp

		input_codes.hpp

		os/file_dialog.cpp
		os/file_dialog.hpp
		os/library_loading.cpp
		os/library_loading.hpp

		initialiser_list.hpp

		type_traits.hpp
)

add_library(toast_lib STATIC) # less problematic...

target_include_directories(toast_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_sources(toast_lib PRIVATE ${SRC})

target_link_libraries(toast_lib PUBLIC glm::glm-header-only)
target_link_libraries(toast_lib PUBLIC fmt::fmt)
target_link_libraries(toast_lib PRIVATE nfd)

add_library(tst::toast_lib ALIAS toast_lib)