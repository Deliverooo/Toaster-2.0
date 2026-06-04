set(SCRIPT_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/script_engine.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/script_object.cpp
)

add_library(ToastScript SHARED)

target_include_directories(ToastScript PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_sources(ToastScript PRIVATE ${SCRIPT_SRC})

target_link_libraries(ToastScript PUBLIC Toaster::ToastLib)

set(MONO_PATH "C:/Program Files/Mono")
target_include_directories(ToastScript PUBLIC "${MONO_PATH}/include/mono-2.0")
find_library(MONO_LIB NAMES mono-2.0-sgen PATHS "${MONO_PATH}/lib")

set(MONO_DLL_PATH "${MONO_PATH}/bin/mono-2.0-sgen.dll")
target_link_libraries(ToastScript PUBLIC ${MONO_LIB})
if (WIN32)
	add_custom_command(TARGET ToastScript POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${MONO_DLL_PATH}"
			$<TARGET_FILE_DIR:ToastScript>)
endif ()

add_library(Toaster::ToastScript ALIAS ToastScript)

add_compile_definitions(ToastScript PUBLIC TST_SCRIPT_BUILD_DLL)
add_compile_definitions(ToastScript PRIVATE TST_SCRIPT_DLL_EXPORT)

install(TARGETS ToastScript
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/toast_script/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toast_script)