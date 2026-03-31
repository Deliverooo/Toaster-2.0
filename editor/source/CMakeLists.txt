set(SRC
		main.cpp
		editor_application.cpp
		editor_application.hpp
		editor_layer.cpp
		editor_layer.hpp
		imgui_layer.cpp
		imgui_layer.hpp
		editor_camera.cpp
		editor_camera.hpp

		panels/scene_hierarchy_panel.cpp
		panels/scene_hierarchy_panel.hpp

		panels/content_browser_panel.cpp
		panels/content_browser_panel.hpp

		ui/ui_utils.cpp
		ui/ui_utils.hpp

		ui/ui_widgets.cpp
		ui/ui_widgets.hpp

		ui/colours.hpp
)

add_executable(toaster_editor ${SRC})

target_link_libraries(toaster_editor PRIVATE Toaster)

target_link_libraries(toaster_editor PRIVATE imgui)
target_link_libraries(toaster_editor PRIVATE ImGuizmo)

