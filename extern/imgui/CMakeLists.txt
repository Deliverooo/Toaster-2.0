project(imgui)


set(IMGUI_SRC
        "imconfig.h"
		"imgui.h"
		"imgui.cpp"
		"imgui_draw.cpp"
		"imgui_internal.h"
		"imgui_tables.cpp"
		"imgui_widgets.cpp"
		"imstb_rectpack.h"
		"imstb_textedit.h"
		"imstb_truetype.h"
		"imgui_demo.cpp" )

set(IMGUI_LIB "")

if(WIN32)

	list(APPEND IMGUI_SRC "backends/imgui_impl_win32.h" "backends/imgui_impl_win32.cpp")
endif ()

if(WITH_DX11_BACKEND)

	list(APPEND IMGUI_SRC "backends/imgui_impl_dx11.h" "backends/imgui_impl_dx11.cpp")
	list(APPEND IMGUI_LIB d3d11 d3dcompiler dxgi)
endif ()

add_library(imgui)

target_sources(imgui PRIVATE ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(imgui PRIVATE ${IMGUI_LIB})