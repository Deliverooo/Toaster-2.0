# Collect sources and headers
set(TST_CORE_SOURCES

		core.cpp

		# Toaster core math sources
		math/vector2.cpp
		math/vector2i.cpp
		math/vector3.cpp
		math/vector3i.cpp
		math/vector4.cpp
		math/vector4i.cpp

		math/math_funcs.cpp
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

if (TST_STATIC_LIBS)
	add_library(ToasterCore STATIC)
else ()
	add_library(ToasterCore SHARED)
	target_compile_definitions(ToasterCore PRIVATE TST_CORE_EXPORT)
endif ()

target_sources(ToasterCore PRIVATE ${TST_CORE_SOURCES})
target_include_directories(ToasterCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(ToasterCore PUBLIC fmt)

toaster_set_target_properties(ToasterCore)
toaster_set_warnings(ToasterCore)

add_library(Toaster::Core ALIAS ToasterCore)