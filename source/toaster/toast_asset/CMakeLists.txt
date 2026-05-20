set(SRC
		asset.hpp

		asset_manager.hpp
		asset_manager.cpp

		texture_asset.cpp
		texture_asset.hpp

		mesh_asset.cpp
		mesh_asset.hpp
)

list(TRANSFORM SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
target_sources(Toaster PRIVATE ${SRC})