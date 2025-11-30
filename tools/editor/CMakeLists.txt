set(TST_EDITOR_SOURCES "")
list(APPEND TST_EDITOR_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/editor_layer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/editor_application.cpp"
)

target_sources(ToasterEditor PRIVATE ${TST_EDITOR_SOURCES})