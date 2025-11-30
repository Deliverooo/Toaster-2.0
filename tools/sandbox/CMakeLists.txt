set(TST_SANDBOX_SOURCES "")
list(APPEND TST_SANDBOX_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/sandbox_layer.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/sandbox_application.cpp"
)

target_sources(Sandbox PRIVATE ${TST_SANDBOX_SOURCES})