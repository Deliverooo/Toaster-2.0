set(SRC

		renderer.hpp
		renderer.cpp

		renderer_2d.cpp
		renderer_2d.hpp

		shader_library.cpp
		shader_library.hpp

		globals.cpp
		globals.hpp

		render_context.cpp
		render_context.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})