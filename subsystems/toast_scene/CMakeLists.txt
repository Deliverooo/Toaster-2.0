set(SCENE_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/scene.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/scene_camera.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/scene_renderer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/scene_serializer.cpp
)

add_library(ToastScene SHARED)

target_include_directories(ToastScene PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(ToastScene PRIVATE ${SCENE_SRC})

target_link_libraries(ToastScene PUBLIC Toaster::ToastRender)
target_link_libraries(ToastScene PUBLIC Toaster::ToastScript)
target_link_libraries(ToastScene PUBLIC EnTT::EnTT) # For the entities
target_link_libraries(ToastScene PRIVATE yaml-cpp)

add_library(Toaster::ToastScene ALIAS ToastScene)

add_compile_definitions(ToastScene PUBLIC TST_SCENE_BUILD_DLL)
add_compile_definitions(ToastScene PRIVATE TST_SCENE_DLL_EXPORT)