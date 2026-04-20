set(SRC
		main.cpp
		client_application.cpp
		client_application.hpp
		client_layer.cpp
		client_layer.hpp
		imgui_layer.cpp
		imgui_layer.hpp
		editor_camera.cpp
		editor_camera.hpp

)

add_executable(toaster_client ${SRC})

target_link_libraries(toaster_client PRIVATE Toaster)
target_link_libraries(toaster_client PRIVATE imgui)

target_link_libraries(toaster_client PRIVATE
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets)


if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
	set(DEBUG_SUFFIX)
	if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
		set(DEBUG_SUFFIX "d")
	endif ()
	set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
	if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
		set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
		if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
			set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
		endif ()
	endif ()
	if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
		add_custom_command(TARGET toaster_client POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E make_directory
				"$<TARGET_FILE_DIR:toaster_client>/plugins/platforms/")
		add_custom_command(TARGET toaster_client POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
				"${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
				"$<TARGET_FILE_DIR:toaster_client>/plugins/platforms/")
	endif ()
	foreach (QT_LIB Core Gui Widgets)
		add_custom_command(TARGET toaster_client POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
				"${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
				"$<TARGET_FILE_DIR:toaster_client>")
	endforeach (QT_LIB)
endif ()
