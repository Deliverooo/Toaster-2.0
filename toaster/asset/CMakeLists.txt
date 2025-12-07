set(TST_ASSET_SOURCES "")
list(APPEND TST_ASSET_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/texture_importer.cpp"
)

target_sources(ToasterEngine PRIVATE ${TST_ASSET_SOURCES})