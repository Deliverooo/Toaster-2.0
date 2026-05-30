add_library(ToastMath INTERFACE)

option(TST_MATH_FORMAT_SPECIALIZATIONS "Provides fmt::format specializations for the math structures" OFF)

target_include_directories(ToastMath INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include/toast_math>)

if (TST_MATH_FORMAT_SPECIALIZATIONS)
	# You better have fmt...
	target_link_libraries(ToastMath INTERFACE fmt::fmt)
	target_compile_definitions(ToastMath INTERFACE -DTST_MATH_FORMAT_SPECIALIZATIONS)
endif ()

add_library(Toaster::ToastMath ALIAS ToastMath)

install(TARGETS ToastMath
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include/toast_math)