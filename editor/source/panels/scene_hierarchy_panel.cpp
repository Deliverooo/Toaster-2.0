#include "scene_hierarchy_panel.hpp"
#include "toaster/toast_scene/components.hpp"

#include "imgui_internal.h"
#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_lib/os/file_dialog.hpp"

#include "../ui/ui_utils.hpp"

namespace toaster
{
	static void drawVec3Ctrl(const std::string &p_label, glm::vec3 *p_vec, const glm::vec3 &p_reset = glm::vec3{1.0f}, float p_column_width = 100.0f,
							 const char *       p_vec1_label                                        = "X", const char *p_vec2_label = "Y", const char *p_vec3_label = "Z")
	{
		const std::string s_vec1_str_id = std::string("##") + p_vec1_label;
		const std::string s_vec2_str_id = std::string("##") + p_vec2_label;
		const std::string s_vec3_str_id = std::string("##") + p_vec3_label;

		ig::PushID(p_label.c_str());
		ig::Columns(2);
		ig::SetColumnWidth(0, p_column_width);
		ig::Text("%s", p_label.c_str());
		ig::NextColumn();

		ig::PushMultiItemsWidths(3, ig::CalcItemWidth());
		{
			ui::ScopedStyle item_spacing{ImGuiStyleVar_ItemSpacing, ImVec2{2, 0}};
			ui::ScopedStyle frame_rounding{ImGuiStyleVar_FrameRounding, 0.5f};

			static ImVec4 s_text_colour = {1.0f, 1.0f, 1.0f, 1.0f};

			const float line_height = ig::GetFrameHeightWithSpacing();
			const auto  button_size = ImVec2{line_height, line_height};

			{
				ui::ScopedColour text_colour{ImGuiCol_Text, s_text_colour};
				ui::ScopedColour button_colour{ImGuiCol_Button, ImColor{0.7f, 0.1f, 0.1f, 1.0f}};
				ui::ScopedColour button_hover_colour{ImGuiCol_Button, ImColor{0.9f, 0.2f, 0.2f, 1.0f}};
				ui::ScopedColour button_active_colour{ImGuiCol_Button, ImColor{0.7f, 0.1f, 0.1f, 1.0f}};

				{
					ui::ScopedStyle rounding{ImGuiStyleVar_FrameRounding, 2.5f};
					if (ig::Button(p_vec1_label, button_size)) { p_vec->x = p_reset.x; }
				}
				ig::SameLine();
				ig::DragFloat(s_vec1_str_id.c_str(), &p_vec->x, 0.1f);
				ig::PopItemWidth();
				ig::SameLine();
			}
			{
				ui::ScopedColour text_colour{ImGuiCol_Text, s_text_colour};
				ui::ScopedColour button_colour{ImGuiCol_Button, ImColor{0.1f, 0.7f, 0.1f, 1.0f}};
				ui::ScopedColour button_hover_colour{ImGuiCol_Button, ImColor{0.2f, 0.9f, 0.2f, 1.0f}};
				ui::ScopedColour button_active_colour{ImGuiCol_Button, ImColor{0.1f, 0.7f, 0.1f, 1.0f}};

				{
					ui::ScopedStyle rounding{ImGuiStyleVar_FrameRounding, 2.5f};
					if (ig::Button(p_vec2_label, button_size)) { p_vec->y = p_reset.y; }
				}
				ig::SameLine();
				ig::DragFloat(s_vec2_str_id.c_str(), &p_vec->y, 0.1f);
				ig::PopItemWidth();
				ig::SameLine();
			}
			{
				ui::ScopedColour text_colour{ImGuiCol_Text, s_text_colour};
				ui::ScopedColour button_colour{ImGuiCol_Button, ImColor{0.1f, 0.1f, 0.7f, 1.0f}};
				ui::ScopedColour button_hover_colour{ImGuiCol_Button, ImColor{0.2f, 0.2f, 0.9f, 1.0f}};
				ui::ScopedColour button_active_colour{ImGuiCol_Button, ImColor{0.1f, 0.1f, 0.7f, 1.0f}};

				{
					ui::ScopedStyle rounding{ImGuiStyleVar_FrameRounding, 2.5f};
					if (ig::Button(p_vec3_label, button_size)) { p_vec->z = p_reset.z; }
				}
				ig::SameLine();
				ig::DragFloat(s_vec3_str_id.c_str(), &p_vec->z, 0.1f);
				ig::PopItemWidth();
			}
		}
		ig::Columns(1);

		ImGui::PopID();
	}

	SceneHierarchyPanel::SceneHierarchyPanel(const RefPtr<Scene> &p_scene) : m_scene(p_scene)
	{
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::setScene(const RefPtr<Scene> &p_scene)
	{
		m_scene = p_scene;
	}

	void SceneHierarchyPanel::onUIRender()
	{
		ig::Begin("Scene Hierarchy");

		auto &reg = m_scene->getRegistry();

		for (auto e: reg.view<entt::entity>())
		{
			Entity entity = {e, m_scene.get()};
			_drawEntityNode(entity);
		}

		if (ig::IsMouseDown(ImGuiMouseButton_Left) && ig::IsWindowHovered())
			m_selectedEntity = {};

		ig::End();

		ig::Begin("Properties");

		if (m_selectedEntity)
		{
			_drawComponents(m_selectedEntity);
		}

		ig::End();
	}

	void SceneHierarchyPanel::_drawEntityNode(Entity p_entity)
	{
		auto &tag_comp = p_entity.getComponent<TagComponent>();

		ImGuiTreeNodeFlags flags = (m_selectedEntity == p_entity) ? ImGuiTreeNodeFlags_Selected : 0 | ImGuiTreeNodeFlags_OpenOnArrow;
		bool               open  = ig::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64>(static_cast<uint32>(p_entity))), flags, "%s", tag_comp.tag.c_str());

		if (ig::IsItemClicked())
		{
			m_selectedEntity = p_entity;
		}

		if (open)
		{
			ig::TreePop();
		}
	}

	void SceneHierarchyPanel::_drawComponents(Entity p_entity)
	{
		{
			auto &tag_comp = p_entity.getComponent<TagComponent>();

			char8 buffer[256]{};
			std::memcpy(buffer, tag_comp.tag.c_str(), tag_comp.tag.size());

			if (ig::InputText("Tag", reinterpret_cast<char *>(buffer), sizeof(buffer)))
			{
				tag_comp.tag = U8String{buffer};
			}
		}
		{
			if (ig::TreeNodeEx(typeid(TransformComponent).name(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto &trans_comp = p_entity.getComponent<TransformComponent>();

				drawVec3Ctrl("Position", &trans_comp.translation, glm::vec3{0.0f}, 75.0f);
				drawVec3Ctrl("Rotation", &trans_comp.rotation, glm::vec3{0.0f}, 75.0f);
				drawVec3Ctrl("Scale", &trans_comp.scale, glm::vec3{1.0f}, 75.0f);

				ig::TreePop();
			}
		}

		if (p_entity.hasComponent<CameraComponent>())
		{
			auto &camera_comp = p_entity.getComponent<CameraComponent>();
			auto &camera      = camera_comp.camera;

			if (ig::TreeNodeEx(typeid(CameraComponent).name(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{
				const char *s_projection_type_names[] = {"Perspective", "Orthographic"};

				const char *current_projection_type = s_projection_type_names[static_cast<int32>(camera.getProjectionType())];
				if (ig::BeginCombo("Projection Type", current_projection_type))
				{
					for (uint32 i{0u}; i < 2; ++i)
					{
						bool selected = current_projection_type == s_projection_type_names[i];
						if (ig::Selectable(s_projection_type_names[i], selected))
						{
							current_projection_type = s_projection_type_names[i];
							camera.setProjectionType(static_cast<SceneCamera::EProjectionType>(i));
						}
						if (selected)
							ig::SetItemDefaultFocus();
					}
					ig::EndCombo();
				}

				if (camera.getProjectionType() == SceneCamera::EProjectionType::ePerspective)
				{
					float fov = glm::degrees(camera.getPerspectiveFov());
					if (ig::DragFloat("Fov", &fov, 0.1f))
						camera.setPerspectiveFov(glm::radians(fov));

					float z_near = camera.getPerspectiveNearClip();
					if (ig::DragFloat("ZNear", &z_near, 0.1f))
						camera.setPerspectiveNearClip(z_near);

					float z_far = camera.getPerspectiveFarClip();
					if (ig::DragFloat("ZFar", &z_far, 0.1f))
						camera.setPerspectiveFarClip(z_far);
				}
				if (camera.getProjectionType() == SceneCamera::EProjectionType::eOrthographic)
				{
					float size = camera.getOrthoSize();
					if (ig::DragFloat("Size", &size, 0.1f))
						camera.setOrthoSize(size);

					float z_near = camera.getOrthoNearClip();
					if (ig::DragFloat("ZNear", &z_near, 0.1f))
						camera.setOrthoNearClip(z_near);

					float z_far = camera.getOrthoFarClip();
					if (ig::DragFloat("ZFar", &z_far, 0.1f))
						camera.setOrthoFarClip(z_far);
				}

				ig::Checkbox("Primary", &camera_comp.primary);
				ig::SetItemTooltip("If true, the camera will be used as the main camera to view the scene from.");

				ig::TreePop();
			}
		}

		if (p_entity.hasComponent<SpriteRendererComponent>())
		{
			auto &sprite_comp = p_entity.getComponent<SpriteRendererComponent>();

			if (ig::TreeNodeEx(typeid(SpriteRendererComponent).name(), ImGuiTreeNodeFlags_DefaultOpen, "Sprite Renderer"))
			{
				ig::ColorEdit4("Colour", &sprite_comp.colour.x);

				ig::DragFloat("Tiling Factor", &sprite_comp.tilingFactor, 0.1f);

				if (ig::Button("File"))
				{
					auto path = os::openFileDialog({{"Image", "png,jpg,jpeg"}});
					if (io::filesystem::exists(path))
					{
						LOG_INFO("{}", path.string());
						sprite_comp.texture = gpu::ITexture2D::create(path);
					}
				}

				ig::TreePop();
			}
		}
	}
}
