add_library(stb STATIC)
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(stb PRIVATE src/stb_impl.c)