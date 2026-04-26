set(SRC
		scene.cpp
		scene.hpp

		components.hpp

		entity.hpp

		scene_camera.hpp
		scene_camera.cpp

		scene_serializer.cpp
		scene_serializer.hpp

		scriptable_entity.hpp

		scene_renderer.cpp
		scene_renderer.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})