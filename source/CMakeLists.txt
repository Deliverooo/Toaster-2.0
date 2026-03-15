add_library(Toaster STATIC)
target_include_directories(Toaster PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(toaster)

target_link_libraries(Toaster PUBLIC glfw)
target_link_libraries(Toaster PUBLIC fmt::fmt)
target_link_libraries(Toaster PUBLIC glm)
target_link_libraries(Toaster PUBLIC stb)
target_link_libraries(Toaster PUBLIC EnTT::EnTT)

target_link_libraries(Toaster PRIVATE assimp::assimp)
