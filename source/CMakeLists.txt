add_library(Toaster STATIC)
target_include_directories(Toaster PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(toaster)

target_link_libraries(Toaster PUBLIC tst::toast_lib)
target_link_libraries(Toaster PUBLIC tst::toast_gpu)

target_link_libraries(Toaster PRIVATE assimp::assimp)
target_link_libraries(Toaster PUBLIC EnTT::EnTT)

if (WIN32)
	target_link_libraries(Toaster PRIVATE dwmapi) # For window colour
endif ()