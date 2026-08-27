add_library(ToastMath INTERFACE)

target_include_directories(ToastMath INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>)

add_library(Toaster::ToastMath ALIAS ToastMath)

if (TOASTER_INSTALL)
	install(TARGETS ToastMath
			EXPORT ToasterTargets
			RUNTIME DESTINATION bin
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
	)
	install(DIRECTORY include/ DESTINATION include)
endif ()