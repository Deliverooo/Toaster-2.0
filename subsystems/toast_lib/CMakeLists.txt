set(SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/toast_assert.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/io/filesystem.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/io/file_stream.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/os/file_dialog.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/os/library_loading.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/os/terminal.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/uuid.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/string.cpp
)

add_library(ToastLib SHARED)

target_include_directories(ToastLib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/toast_lib>)

target_sources(ToastLib PRIVATE ${SRC})

set(TST_MATH_FORMAT_SPECIALIZATIONS ON CACHE BOOL "" FORCE)

target_link_libraries(ToastLib PUBLIC ToastMath)
target_link_libraries(ToastLib PUBLIC fmt::fmt)
target_link_libraries(ToastLib PRIVATE nfd)

add_library(Toaster::ToastLib ALIAS ToastLib)

add_compile_definitions(ToastLib PUBLIC TST_LIB_BUILD_DLL)
add_compile_definitions(ToastLib PRIVATE TST_LIB_DLL_EXPORT)

install(TARGETS ToastLib
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)