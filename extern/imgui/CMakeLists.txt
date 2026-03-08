project(imgui)

option(WITH_OPENGL "Build ImGui with OpenGL3 support" ON)
option(WITH_GLFW "Build ImGui with OpenGL3 support" ON)

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

add_library(imgui STATIC)

if(WITH_OPENGL)
	list(APPEND IMGUI_SRC "backends/imgui_impl_opengl3.h" "backends/imgui_impl_opengl3.cpp")
endif ()
if(WITH_GLFW)
	list(APPEND IMGUI_SRC "backends/imgui_impl_glfw.h" "backends/imgui_impl_glfw.cpp")
	target_link_libraries(imgui PUBLIC glfw)
endif ()


target_sources(imgui PRIVATE ${IMGUI_SRC})
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

