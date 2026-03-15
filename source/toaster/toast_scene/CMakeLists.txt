set(SRC
		scene.cpp
		scene.hpp

		components.hpp

		entity.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})