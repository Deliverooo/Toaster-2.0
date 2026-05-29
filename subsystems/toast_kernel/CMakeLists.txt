set(KERNEL_SRC
		${CMAKE_CURRENT_SOURCE_DIR}/src/application.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/input.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/layer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp
)

add_library(ToastKernel SHARED)

target_include_directories(ToastKernel PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(ToastKernel PRIVATE ${KERNEL_SRC})

target_link_libraries(ToastKernel PUBLIC Toaster::ToastScene)
target_link_libraries(ToastKernel PUBLIC argparse)
target_link_libraries(ToastKernel PRIVATE glfw)
target_link_libraries(ToastKernel PRIVATE dwmapi)

add_library(Toaster::ToastKernel ALIAS ToastKernel)

add_compile_definitions(ToastKernel PUBLIC TST_KERNEL_BUILD_DLL)
add_compile_definitions(ToastKernel PRIVATE TST_KERNEL_DLL_EXPORT)