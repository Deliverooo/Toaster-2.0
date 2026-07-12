set(RENDER_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/globals.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/render_attachment.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/graphics_state.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/shader_compiler.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/render_context.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/dynamic_renderer_2d.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/dynamic_material.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/shader_reflection.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/dynamic_mesh.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/constant_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/uniform_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/storage_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/shader_library.cpp
)

add_library(ToastRender SHARED)


target_include_directories(ToastRender PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_sources(ToastRender PRIVATE ${RENDER_SRC})

#find_package(directx-dxc CONFIG REQUIRED)
set(DXC_COMPILER_DLL "$ENV{VULKAN_SDK}/Bin/dxcompiler.dll")
if (EXISTS ${DXC_COMPILER_DLL})
	add_custom_command(TARGET ToastRender POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${DXC_COMPILER_DLL}"
			"$<TARGET_FILE_DIR:ToastRender>"
			COMMENT "Copying directX compiler Dll..."
	)
endif ()

target_link_libraries(ToastRender PRIVATE dxcompiler d3dcompiler dxgi d3d12)
#target_link_libraries(ToastRender PRIVATE dxil)

target_link_libraries(ToastRender PUBLIC Toaster::ToastGPU)
target_link_libraries(ToastRender PUBLIC meshoptimizer)
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