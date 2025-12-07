set(TST_RENDERER_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/render_command_queue.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/renderer_2d.cpp"
)

target_sources(ToasterEngine PRIVATE ${TST_RENDERER_SOURCES})
