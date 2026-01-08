set(SRC

		logging.cpp
		logging.hpp

		toast_assert.cpp
		toast_assert.h

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

		util_defines.hpp

		toast_exception.cpp
		toast_exception.hpp
)

add_library(toast_lib STATIC)

target_include_directories(toast_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(toast_lib PRIVATE ${SRC})

target_link_libraries(toast_lib PUBLIC glm)
target_link_libraries(toast_lib PUBLIC fmt::fmt)

add_library(tst::toast_lib ALIAS toast_lib)