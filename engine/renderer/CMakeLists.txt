set(TST_RENDERER_SOURCES "")
list(APPEND TST_RENDERER_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/render_command_queue.cpp"
)

target_sources(ToasterEngine PRIVATE ${TST_RENDERER_SOURCES})