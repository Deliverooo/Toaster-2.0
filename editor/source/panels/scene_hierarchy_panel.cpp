#include "scene_hierarchy_panel.hpp"
#include "toaster/toast_scene/components.hpp"

#include <imgui.h>
namespace ig = ImGui;

namespace toaster
{
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
			Entity entity   = {e, m_scene.get()};
			auto & tag_comp = entity.getComponent<TagComponent>();
			ig::Text("%s", tag_comp.tag.c_str());
		}

		ig::End();
	}
}
