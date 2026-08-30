add_library(stb STATIC)

target_include_directories(stb PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>)
target_sources(stb PRIVATE src/stb_impl.c)

install(TARGETS stb
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)