set(TST_UI_SOURCES "")
list(APPEND TST_UI_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui_layer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui_renderer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui_build.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/imgui_fonts.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/ui_core.cpp"
)

target_sources(ToasterEngine PRIVATE ${TST_UI_SOURCES})