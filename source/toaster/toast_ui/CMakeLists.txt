
set(UI_SRC
		toast_ui.hpp

		breadcrumb_context.hpp
		breadcrumb_context.cpp
)

add_library(toast_ui SHARED)

target_include_directories(toast_ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_sources(toast_ui PRIVATE ${UI_SRC})

target_link_libraries(toast_ui PUBLIC tst::toast_gpu)

add_library(tst::toast_ui ALIAS toast_ui)
add_dependencies(toast_ui tst::toast_ui)

add_compile_definitions(toast_ui PUBLIC TST_UI_BUILD_DLL)
add_compile_definitions(toast_ui PRIVATE TST_UI_DLL_EXPORT)