using System.Runtime.CompilerServices;

namespace Toaster;

public abstract class Component
{
	public Entity EntityParent { get; internal set; } = null!;
}

public class TagComponent : Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetTag(uint p_entity_id, out string p_out_tag);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetTag(uint p_entity_id, ref string p_tag);

	public string Tag
	{
		get
		{
			GetTag(EntityParent.Id, out var tag);
			return tag;
		}
		set => SetTag(EntityParent.Id, ref value);
	}
}

public class TransformComponent : Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetTranslation(uint p_entity_id, out Vec3 p_out_translation);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetTranslation(uint p_entity_id, ref Vec3 p_translation);

	public Vec3 Translation
	{
		get
		{
			GetTranslation(EntityParent.Id, out var translation);
			return translation;
		}
		set => SetTranslation(EntityParent.Id, ref value);
	}
}

public class SpriteRendererComponent : Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetColour(uint p_entity_id, out Vec4 p_out_colour);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetColour(uint p_entity_id, ref Vec4 p_colour);

	public Vec4 Colour
	{
		get
		{
			GetColour(EntityParent.Id, out var colour);
			return colour;
		}
		set => SetColour(EntityParent.Id, ref value);
	}
}

public class MeshComponent : Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern bool HasMaterialInternal(uint p_entity_id, uint p_index);

	public bool HasMaterial(uint p_index)
	{
		return HasMaterialInternal(EntityParent.Id, p_index);
	}

	public Material GetMaterial(uint p_index)
	{
		if (!HasMaterial(p_index))
			return null;

		return new Material(EntityParent.Id, p_index);
	}
}