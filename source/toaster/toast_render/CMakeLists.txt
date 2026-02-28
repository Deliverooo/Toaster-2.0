set(SRC

		renderer.hpp
		renderer.cpp

		mesh.cpp
		mesh.hpp

		ortho_camera.hpp
		ortho_camera.cpp

		render_command.cpp
		render_command.hpp

		renderer_2d.cpp
		renderer_2d.hpp

		camera.hpp

		shader_library.cpp
		shader_library.hpp

		material.cpp
		material.hpp

		globals.cpp
		globals.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})