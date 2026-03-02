add_library(miniaudio STATIC)
target_include_directories(miniaudio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(miniaudio PRIVATE miniaudio.c)