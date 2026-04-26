set(SRC
		script_engine.cpp
		script_engine.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})