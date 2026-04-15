set(SRC
		asset.hpp

		asset_manager.hpp
		asset_manager.cpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})