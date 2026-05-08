#include "scene_hierarchy_panel.hpp"
#include "toaster/toast_scene/components.hpp"

#include "imgui_internal.h"
#include "toaster/toast_lib/logging.hpp"
#include "toaster/toast_lib/os/file_dialog.hpp"

#include "../ui/ui_utils.hpp"
#include "../ui/ui_widgets.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "toast_gpu/vk/vk_logical_device.hpp"
#include "toast_render/globals.hpp"

namespace toaster
{
	static auto drawVec3Ctrl(const String &p_label, glm::vec3 *p_vec, const glm::vec3 &p_reset = glm::vec3{1.0f}, const char *p_vec1_label = "X",
							 const char *  p_vec2_label                                        = "Y", const char *            p_vec3_label = "Z") -> bool
	{
		const String vec1_str_id = String("##") + p_vec1_label;
		const String vec2_str_id = String("##") + p_vec2_label;
		const String vec3_str_id = String("##") + p_vec3_label;

		ImGuiIO &io        = ig::GetIO();
		auto     bold_font = io.Fonts->Fonts[1];

		bool ret{false};

		ig::PushID(p_label.c_str());

		ig::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5f);
		ret |= ui::dragFloatWithReset(p_label + " " + p_vec1_label, &p_vec->x, vec1_str_id.c_str(), 0.1f, 0, 0, "%.3f", p_reset.x);
		ret |= ui::dragFloatWithReset(p_vec2_label, &p_vec->y, vec2_str_id.c_str(), 0.1f, 0, 0, "%.3f", p_reset.y);
		ret |= ui::dragFloatWithReset(p_vec3_label, &p_vec->z, vec3_str_id.c_str(), 0.1f, 0, 0, "%.3f", p_reset.z);
		ig::PopStyleVar();

		ig::PopID();

		return ret;
	}

	template<typename Type, bool Removable = true, typename UIFunc>
	static auto drawComponent(const String &p_name, Entity p_entity, UIFunc p_func, void *p_caller_id) -> void
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
				{
					SceneHierarchyPanel *caller{(SceneHierarchyPanel *) p_caller_id};
					caller->m_device->getVulkanLogicalDevice().waitIdle();
					comp.reset();
				}

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

	SceneHierarchyPanel::SceneHierarchyPanel(gpu::VKLogicalDevice *p_device, const RefPtr<Scene> &p_scene) : m_device(p_device), m_scene(p_scene)
	{
	}

	SceneHierarchyPanel::~SceneHierarchyPanel() = default;

	auto SceneHierarchyPanel::setScene(const RefPtr<Scene> &p_scene) -> void
	{
		m_scene          = p_scene;
		m_selectedEntity = {};
	}

	auto SceneHierarchyPanel::onUIRender(uint32 p_frame_index) -> void
	{
		ig::Begin("Scene Hierarchy");

		auto &reg = m_scene->getRegistry();

		for (auto e: reg.view<entt::entity>())
		{
			Entity entity = {e, m_scene.get()};
			_drawEntityNode(entity, p_frame_index);
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
			_drawComponents(m_selectedEntity, p_frame_index);
		}

		ig::End();
	}

	auto SceneHierarchyPanel::getSelectedEntity() const -> Entity
	{
		return m_selectedEntity;
	}

	auto SceneHierarchyPanel::setSelectedEntity(Entity p_entity) -> void
	{
		m_selectedEntity = p_entity;
	}

	auto SceneHierarchyPanel::_drawEntityNode(Entity p_entity, uint32 p_frame_index) -> void
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

	auto SceneHierarchyPanel::_drawComponents(Entity p_entity, uint32 p_frame_index) -> void
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

			if (ig::MenuItem("Mesh"))
			{
				m_selectedEntity.addComponent<MeshComponent>();
				ig::CloseCurrentPopup();
			}

			if (ig::MenuItem("Camera"))
			{
				m_selectedEntity.addComponent<CameraComponent>();
				ig::CloseCurrentPopup();
			}

			if (ig::MenuItem("Directional Light"))
			{
				m_selectedEntity.addComponent<DirectionalLightComponent>();
				ig::CloseCurrentPopup();
			}

			if (ig::MenuItem("Point Light"))
			{
				m_selectedEntity.addComponent<PointLightComponent>();
				ig::CloseCurrentPopup();
			}

			if (ig::MenuItem("Spot Light"))
			{
				m_selectedEntity.addComponent<SpotLightComponent>();
				ig::CloseCurrentPopup();
			}

			ig::EndPopup();
		}

		drawComponent<TransformComponent, false>("Transform", p_entity, [](TransformComponent &p_comp)
		{
			drawVec3Ctrl("Position", &p_comp.translation, glm::vec3{0.0f});
			ig::Separator();

			glm::vec3 rotation_euler{glm::eulerAngles(p_comp.rotation)};
			if (drawVec3Ctrl("Rotation", &rotation_euler, glm::vec3{0.0f}))
				p_comp.rotation = glm::quat{rotation_euler};
			ig::Separator();
			drawVec3Ctrl("Scale", &p_comp.scale, glm::vec3{1.0f});
			ig::Separator();
		}, this);

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
		}, this);

		drawComponent<SpriteRendererComponent>("Sprite Renderer", p_entity, [this](SpriteRendererComponent &p_comp)
		{
			ui::colourEdit4("Colour", &p_comp.colour.x);

			ig::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.5);
			ui::dragFloat("Tiling Factor", &p_comp.tilingFactor, "##Tiling_Factor", 0.1f);
			ig::PopStyleVar();

			ig::Text("Texture:");
			if (ig::Button("File", ImVec2{ig::GetContentRegionAvail().x, 0}))
			{
				auto path = os::openFileDialog({{"Texture", "png,jpg,jpeg"}});
				if (io::filesystem::exists(path))
				{
					LOG_INFO("{}", path.string());

					gpu::TextureSpecInfo texture_spec_info{};
					p_comp.texture = m_device->alloc<gpu::VKTexture2D>(texture_spec_info, path);
				}
			}
		}, this);

		drawComponent<MeshComponent>("Mesh", p_entity, [this,p_frame_index](MeshComponent &p_comp)
		{
			ig::Text("Mesh source file:");
			if (ig::Button("File", ImVec2{ig::GetContentRegionAvail().x, 0}))
			{
				auto path = os::openFileDialog({{"Mesh", "fbx,obj,glb"}});
				if (io::filesystem::exists(path))
				{
					LOG_INFO("{}", path.string());

					auto geometry_shader{Globals::getShaderLibrary().get("Geometry")};
					p_comp.mesh = m_device->alloc<gpu::VKMesh>(path, geometry_shader);
				}
			}

			if (p_comp.mesh)
			{
				for (auto &mat: p_comp.mesh->getMaterials())
					_drawMaterial(p_frame_index, mat);
			}
		}, this);

		drawComponent<DirectionalLightComponent>("Directional Light", p_entity, [](DirectionalLightComponent &p_comp)
		{
			ui::colourEdit3("Radiance", &p_comp.radiance.x);
			ui::dragFloat("Multiplier", &p_comp.multiplier, "##Multiplier", 0.01f);
		}, this);

		drawComponent<PointLightComponent>("Point Light", p_entity, [](PointLightComponent &p_comp)
		{
			ui::colourEdit3("Radiance", glm::value_ptr(p_comp.radiance));
			ui::dragFloat("Multiplier", &p_comp.multiplier, "##Multiplier", 0.01f);
		}, this);

		drawComponent<SpotLightComponent>("Spot Light", p_entity, [](SpotLightComponent &p_comp)
		{
			ui::colourEdit3("Radiance", glm::value_ptr(p_comp.radiance));
			ui::dragFloat("Multiplier", &p_comp.multiplier, "##Multiplier", 0.01f);
			ig::Separator();
			ui::dragFloat("Angle", &p_comp.angle, "##Angle", 0.05f);
			ui::dragFloat("Range", &p_comp.range, "##Range", 0.05f);
			ui::dragFloat("Falloff", &p_comp.falloff, "##Falloff", 0.05f);
		}, this);
	}

	auto SceneHierarchyPanel::_drawMaterial(uint32 p_frame_index, const RefPtr<gpu::VKMaterial> &p_mat) -> void
	{
		ig::PushID(p_mat->getName().c_str());
		ig::Text("Material: %s", p_mat->getName().c_str());

		auto albedo_map{p_mat->getResource<gpu::VKTexture2D>("u_AlbedoTexture")};
		if (albedo_map)
		{
			ig::Image(ImTextureRef(p_mat->getDescriptorSet(p_frame_index)), ImVec2{100, 100}, ImVec2{0, 0}, ImVec2{1, 1});
		}

		if (ig::Button("Albedo texture", ImVec2{ig::GetContentRegionAvail().x, 0}))
		{
			auto path = os::openFileDialog({{"Albedo", "png,jpg,bmp"}});
			if (io::filesystem::exists(path))
			{
				LOG_INFO("{}", path.string());
				p_mat->set("u_AlbedoTexture", m_device->alloc<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, path));
			}
		}

		if (ig::Button("Normal texture", ImVec2{ig::GetContentRegionAvail().x, 0}))
		{
			auto path = os::openFileDialog({{"Normal", "png,jpg,bmp"}});
			if (io::filesystem::exists(path))
			{
				LOG_INFO("{}", path.string());
				p_mat->set("u_NormalTexture", m_device->alloc<gpu::VKTexture2D>(gpu::TextureSpecInfo{}, path));
				p_mat->set("u_Material.hasNormalMap", 1u);
			}
		}

		ig::Separator();
		glm::vec3 &colour{p_mat->get<glm::vec3>("u_Material.albedoColour")};
		if (ui::colourEdit3("Colour", glm::value_ptr(colour)))
			p_mat->set("u_Material.albedoColour", colour);

		ig::Separator();
		float32 &roughness{p_mat->get<float32>("u_Material.roughness")};
		if (ui::dragFloat("Roughness", &roughness, "##Roughness", 0.01f, 0.001f, 1.0f))
			p_mat->set("u_Material.roughness", roughness);

		ig::Separator();
		float32 &metalness{p_mat->get<float32>("u_Material.metalness")};
		if (ui::dragFloat("Metalness", &metalness, "##Metalness", 0.01f, 0.001f, 1.0f))
			p_mat->set("u_Material.metalness", metalness);

		ig::Separator();
		ig::PopID();
	}
}
