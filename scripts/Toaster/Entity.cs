namespace Toaster;

public abstract class Entity
{
	protected Entity()
	{
		Id = 0;
	}

	internal Entity(uint p_id)
	{
		Id = p_id;
	}

	public readonly uint Id;

	protected abstract void OnCreate();
	protected abstract void OnUpdate(float p_dt);

	private bool HasComponent<T>() where T : Component, new()
	{
		return InternalCalls.HasComponent(Id, typeof(T));
	}

	protected T GetComponent<T>() where T : Component, new()
	{
		return !HasComponent<T>() ? null : new T() { EntityParent = this };
	}

	protected T AddComponent<T>() where T : Component, new()
	{
		if (HasComponent<T>())
			return null!;

		InternalCalls.AddComponent(Id, typeof(T));
		return new T() { EntityParent = this };
	}
}