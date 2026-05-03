namespace Toaster;

public class Entity
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