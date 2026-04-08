#include "scene_hierarchy_panel.hpp"
#include "toaster/toast_scene/components.hpp"

#include "imgui_internal.h"
#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_lib/os/file_dialog.hpp"

#include "../ui/ui_utils.hpp"
#include "../ui/ui_widgets.hpp"

namespace toaster
{
	static void drawVec3Ctrl(const String &p_label, glm::vec3 *p_vec, const glm::vec3 &p_reset = glm::vec3{1.0f}, const char *p_vec1_label = "X",
							 const char *  p_vec2_label                                        = "Y", const char *            p_vec3_label = "Z")
	{
		const String vec1_str_id = String("##") + p_vec1_label;
		const String vec2_str_id = String("##") + p_vec2_label;
		const String vec3_str_id = String("##") + p_vec3_label;

		ImGuiIO &io        = ig::GetIO();
		auto     bold_font = io.Fonts->Fonts[1];

		ig::PushID(p_label.c_str());

		ig::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);
		ui::dragFloatWithReset(p_label + " " + p_vec1_label, &p_vec->x, vec1_str_id.c_str(), 0.1f, 0, 0, "%.3f", p_reset.x);
		ui::dragFloatWithReset(p_vec2_label, &p_vec->y, vec2_str_id.c_str(), 0.1f, 0, 0, "%.3f", p_reset.y);
		ui::dragFloatWithReset(p_vec3_label, &p_vec->z, vec3_str_id.c_str(), 0.1f, 0, 0, "%.3f", p_reset.z);
		ig::PopStyleVar();

		ig::PopID();
	}

	template<typename Type, bool Removable = true, typename UIFunc>
	static void drawComponent(const String &p_name, Entity p_entity, UIFunc p_func)
	{
		ig::PushID(typeid(Type).hash_code());

		if (p_entity.hasComponent<Type>())
		{
			auto &comp = p_entity.getComponent<Type>();

			ImGuiIO &io = ig::GetIO();

			ImVec2 content_region = ig::GetContentRegionAvail();
			ig::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});

			float line_height = ig::GetFrameHeight();

			float button_width = line_height;
			ig::Separator();

			ig::PushFont(io.Fonts->Fonts[1]);

			bool open = ig::TreeNodeEx(typeid(Type).name(),
									   ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding,
									   "%s", p_name.c_str());
			ig::PopStyleVar(); // ImGuiStyleVar_FramePadding
			ig::SameLine(content_region.x - line_height);
			ig::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);

			if (ig::Button("+", ImVec2{button_width, button_width}))
				ig::OpenPopup("Component_Settings");

			ig::PopFont();
			ig::PopStyleVar(); // ImGuiStyleVar_FrameRounding

			bool remove_component{false};
			if (ig::BeginPopup("Component_Settings"))
			{
				if constexpr (Removable)
				{
					if (ig::MenuItem("Remove Component"))
						remove_component = true;
				}

				if (ig::MenuItem("Reset"))
					comp.reset();

				ig::EndPopup();
			}
			if (open)
			{
				p_func(comp);

				ig::TreePop();
			}
			if (remove_component)
				p_entity.removeComponent<Type>();
		}
		ig::PopID();
	}

	SceneHierarchyPanel::SceneHierarchyPanel(gpu::VKGPUContext *p_ctx,const RefPtr<Scene> &p_scene) : m_ctx(p_ctx), m_scene(p_scene)
	{
	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::setScene(const RefPtr<Scene> &p_scene)
	{
		m_scene          = p_scene;
		m_selectedEntity = {};
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

		if (ig::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ig::MenuItem("Add Entity"))
			{
				m_scene->createEntity();
			}
			ig::EndPopup();
		}

		ig::End();

		ig::Begin("Properties");

		if (m_selectedEntity)
		{
			_drawComponents(m_selectedEntity);
		}

		ig::End();
	}

	Entity SceneHierarchyPanel::getSelectedEntity() const
	{
		return m_selectedEntity;
	}

	void SceneHierarchyPanel::setSelectedEntity(Entity p_entity)
	{
		m_selectedEntity = p_entity;
	}

	void SceneHierarchyPanel::_drawEntityNode(Entity p_entity)
	{
		auto &tag_comp = p_entity.getComponent<TagComponent>();

		ImGuiTreeNodeFlags flags = ((m_selectedEntity == p_entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow |
								   ImGuiTreeNodeFlags_SpanAvailWidth;
		bool open = ig::TreeNodeEx(reinterpret_cast<void *>(static_cast<uint64>(static_cast<uint32>(p_entity))), flags, "%s", tag_comp.tag.c_str());

		if (ig::IsItemClicked())
		{
			m_selectedEntity = p_entity;
		}

		bool delete_entity{false};
		if (ig::BeginPopupContextItem())
		{
			if (ig::MenuItem("Delete Entity"))
				delete_entity = true;
			ig::EndPopup();
		}

		if (open)
		{
			ig::TreePop();
		}

		if (delete_entity)
		{
			m_scene->destroyEntity(p_entity);
			if (p_entity == m_selectedEntity)
				m_selectedEntity = {};
		}
	}

	void SceneHierarchyPanel::_drawComponents(Entity p_entity)
	{
		{
			auto &tag_comp = p_entity.getComponent<TagComponent>();

			char buffer[256]{};
			std::memcpy(buffer, tag_comp.tag.c_str(), tag_comp.tag.size());

			if (ig::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag_comp.tag = String{buffer};
			}
		}
		ig::SameLine();

		ImVec2 size = {ig::GetContentRegionAvail().x, ig::GetFrameHeight()};

		if (ig::Button("Add Component", size))
			ig::OpenPopup("Add_Component");

		if (ig::BeginPopup("Add_Component"))
		{
			if (ig::MenuItem("Sprite Renderer"))
			{
				m_selectedEntity.addComponent<SpriteRendererComponent>();
				ig::CloseCurrentPopup();
			}

			if (ig::MenuItem("Camera"))
			{
				m_selectedEntity.addComponent<CameraComponent>();
				ig::CloseCurrentPopup();
			}

			ig::EndPopup();
		}

		drawComponent<TransformComponent, false>("Transform", p_entity, [](TransformComponent &p_comp)
		{
			drawVec3Ctrl("Position", &p_comp.translation, glm::vec3{0.0f});
			ig::Separator();
			drawVec3Ctrl("Rotation", &p_comp.rotation, glm::vec3{0.0f});
			ig::Separator();
			drawVec3Ctrl("Scale", &p_comp.scale, glm::vec3{1.0f});
			ig::Separator();
		});

		drawComponent<CameraComponent>("Camera", p_entity, [](CameraComponent &p_comp)
		{
			auto &camera = p_comp.camera;

			const char *s_projection_type_names[] = {"Perspective", "Orthographic"};

			const char *current_projection_type = s_projection_type_names[static_cast<int32>(camera.getProjectionType())];
			if (ui::beginCombo("Projection Type", current_projection_type))
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
				ui::endCombo();
			}
			ig::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5);

			if (camera.getProjectionType() == SceneCamera::EProjectionType::ePerspective)
			{
				float fov = glm::degrees(camera.getPerspectiveFov());
				if (ui::dragFloat("Fov", &fov, "##Fov", 0.1f))
					camera.setPerspectiveFov(glm::radians(fov));

				float z_near = camera.getPerspectiveNearClip();
				if (ui::dragFloat("ZNear", &z_near, "##ZNear", 0.1f))
					camera.setPerspectiveNearClip(z_near);

				float z_far = camera.getPerspectiveFarClip();
				if (ui::dragFloat("ZFar", &z_far, "##ZFar", 0.1f))
					camera.setPerspectiveFarClip(z_far);
			}
			if (camera.getProjectionType() == SceneCamera::EProjectionType::eOrthographic)
			{
				float size = camera.getOrthoSize();
				if (ui::dragFloat("Size", &size, "##Size", 0.1f))
					camera.setOrthoSize(size);

				float z_near = camera.getOrthoNearClip();
				if (ui::dragFloat("ZNear", &z_near, "##ZNear", 0.1f))
					camera.setOrthoNearClip(z_near);

				float z_far = camera.getOrthoFarClip();
				if (ui::dragFloat("ZFar", &z_far, "##ZFar", 0.1f))
					camera.setOrthoFarClip(z_far);
			}

			ui::checkbox("Primary", &p_comp.primary);
			ig::PopStyleVar();

			ig::SetItemTooltip("If true, the camera will be used as the main camera to view the scene from.");
		});

		drawComponent<SpriteRendererComponent>("Sprite Renderer", p_entity, [this](SpriteRendererComponent &p_comp)
		{
			ui::colourEdit4("Colour", &p_comp.colour.x);

			ig::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5);
			ui::dragFloat("Tiling Factor", &p_comp.tilingFactor, "##Tiling_Factor", 0.1f);
			ig::PopStyleVar();

			// ig::SetNextItemWidth(ig::GetContentRegionAvail().x);
			if (ig::Button("File", ImVec2{ig::GetContentRegionAvail().x, 0}))
			{
				auto path = os::openFileDialog({{"Image", "png,jpg,jpeg"}});
				if (io::filesystem::exists(path))
				{
					LOG_INFO("{}", path.string());
					gpu::TextureSpecInfo texture_spec{};

					p_comp.texture = make_reference<gpu::VKTexture2D>(m_ctx, texture_spec, path);
				}
			}
		});
	}
}
