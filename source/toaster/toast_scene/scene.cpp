#include "scene.hpp"
#include "entity.hpp"

namespace toaster
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::onUpdate(const RefPtr<Renderer2D> &p_renderer_2d, float32 p_dt)
	{
		auto group = m_registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity: group)
		{
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			if (sprite.texture)
				p_renderer_2d->submitQuad(transform.transform, sprite.texture, sprite.colour);
			else
				p_renderer_2d->submitQuad(transform.transform, sprite.colour);
		}
	}

	Entity Scene::createEntity(const std::wstring &p_name)
	{
		auto entity = Entity{m_registry.create(), this};

		entity.addComponent<TransformComponent>();
		if (!p_name.empty())
		{
			entity.addComponent<TagComponent>(p_name);
		}

		return entity;
	}
}
