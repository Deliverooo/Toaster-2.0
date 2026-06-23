set(ASSET_SRC
		src/texture_serializer.cpp
)

add_library(ToastAsset SHARED)

target_include_directories(ToastAsset PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_sources(ToastAsset PRIVATE ${ASSET_SRC})

target_link_libraries(ToastAsset PUBLIC Toaster::ToastGPU)

add_library(Toaster::ToastAsset ALIAS ToastAsset)

add_compile_definitions(ToastAsset PUBLIC TST_ASSET_BUILD_DLL)
add_compile_definitions(ToastAsset PRIVATE TST_ASSET_DLL_EXPORT)

install(TARGETS ToastAsset
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/toast_asset/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toast_asset)