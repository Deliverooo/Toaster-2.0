set(RENDER_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/globals.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/material.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/mesh.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/render_attachment.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/descriptor_set_manager.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/render_context.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/renderer_2d.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/render_pass.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/compute_pass.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/shader_library.cpp
)

add_library(ToastRender SHARED)


target_include_directories(ToastRender PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_sources(ToastRender PRIVATE ${RENDER_SRC})

target_link_libraries(ToastRender PUBLIC Toaster::ToastGPU)
target_link_libraries(ToastRender PRIVATE assimp::assimp)
add_dependencies(ToastRender Toaster::ToastShaders)

add_library(Toaster::ToastRender ALIAS ToastRender)

add_compile_definitions(ToastRender PUBLIC TST_RENDER_BUILD_DLL)
add_compile_definitions(ToastRender PRIVATE TST_RENDER_DLL_EXPORT)

install(TARGETS ToastRender
		EXPORT ToasterTargets
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
)
install(DIRECTORY include/toast_render/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/toast_render)