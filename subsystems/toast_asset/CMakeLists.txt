set(RENDER_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/texture.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/mesh.cpp
)

if (TOASTER_BUILD_SHARED_LIBS)
	add_library(ToastAsset SHARED)
else ()
	add_library(ToastAsset STATIC)
endif ()

target_include_directories(ToastAsset PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>)

target_sources(ToastAsset PRIVATE ${RENDER_SRC})
target_link_libraries(ToastAsset PUBLIC Toaster::ToastRender assimp::assimp stb)

add_library(Toaster::ToastAsset ALIAS ToastAsset)

if (TOASTER_BUILD_SHARED_LIBS)
	add_compile_definitions(ToastAsset PUBLIC TST_ASSET_BUILD_DLL)
	add_compile_definitions(ToastAsset PRIVATE TST_ASSET_DLL_EXPORT)
endif ()

if (TOASTER_INSTALL)
	install(TARGETS ToastAsset
			EXPORT ToasterTargets
			RUNTIME DESTINATION bin
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
	)
	install(DIRECTORY include/ DESTINATION include)

endif ()