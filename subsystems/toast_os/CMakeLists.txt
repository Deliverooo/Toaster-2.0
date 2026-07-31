set(SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp
)

add_library(ToastOS SHARED)

target_include_directories(ToastOS PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_sources(ToastOS PRIVATE ${SRC})

add_library(Toaster::ToastOS ALIAS ToastOS)

add_compile_definitions(ToastOS PUBLIC TST_OS_BUILD_DLL)
add_compile_definitions(ToastOS PRIVATE TST_OS_DLL_EXPORT)

install(TARGETS ToastOS
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/toast_os/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toast_os)