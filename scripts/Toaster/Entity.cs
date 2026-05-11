using System.Runtime.CompilerServices;

namespace Toaster;

public class Entity
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern bool HasComponentInternal(ulong p_entity_id, Type p_component_type);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void AddComponentInternal(ulong p_entity_id, Type p_component_type);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern ulong GetEntityByNameInternal(string p_name);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern object GetScriptInstance(ulong p_entity_id);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern ulong CreateEntityInternal(string p_name);

	protected Entity() { Id = 0; }

	internal Entity(ulong p_id) { Id = p_id; }

	public readonly ulong Id;

	protected virtual void OnCreate() { }
	protected virtual void OnUpdate(float p_dt) { }

	public bool HasComponent<T>() where T : Component, new() { return HasComponentInternal(Id, typeof(T)); }

	public T GetComponent<T>() where T : Component, new() { return !HasComponent<T>() ? null : new T() { EntityParent = this }; }

	public T AddComponent<T>() where T : Component, new()
	{
		if (HasComponent<T>())
			return null!;

		AddComponentInternal(Id, typeof(T));
		return new T() { EntityParent = this };
	}

	public Entity GetEntityByName(string p_name)
	{
		ulong entity_id = GetEntityByNameInternal(p_name);
		if (entity_id == 0)
			return null;
		return new Entity(entity_id);
	}

	public T As<T>() where T : Entity
	{
		object instance = GetScriptInstance(Id);
		return instance as T;
	}

	public static Entity CreateEntity(string p_name)
	{
		ulong entity_id = CreateEntityInternal(p_name);
		return new Entity(entity_id);
	}
}