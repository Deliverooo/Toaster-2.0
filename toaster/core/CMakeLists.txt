# Collect sources and headers
set(TST_CORE_SOURCES


		io/stream_reader.cpp
		io/stream_writer.cpp

		log.cpp

		layer_stack.cpp

		thread.cpp

		ref_ptr.cpp

		filesystem/filesystem.cpp

		math/matrix.cpp
)

set(TST_CORE_HEADERS

		core_defines.hpp
		error_macros.hpp

		# Toaster core math headers

		math/vector2.hpp
		math/vector2i.hpp
		math/vector3.hpp
		math/vector3i.hpp
		math/vector4.hpp
		math/vector4i.hpp
		math/rect.hpp
)

add_library(ToasterCore STATIC)


target_sources(ToasterCore PRIVATE ${TST_CORE_SOURCES})
target_include_directories(ToasterCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(ToasterCore PUBLIC fmt spdlog)

toaster_set_target_properties(ToasterCore)
toaster_set_warnings(ToasterCore)

add_library(Toaster::Core ALIAS ToasterCore)
