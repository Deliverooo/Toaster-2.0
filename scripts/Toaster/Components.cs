using System.Runtime.CompilerServices;

namespace Toaster;

public abstract class Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void ResetInternal(uint p_entity_id, Type p_type);

	public void Reset() { ResetInternal(EntityParent.Id, this.GetType()); }

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

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetRotation(uint p_entity_id, out Quat p_out_rotation);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetRotation(uint p_entity_id, ref Quat p_rotation);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetScale(uint p_entity_id, out Vec3 p_out_scale);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetScale(uint p_entity_id, ref Vec3 p_scale);

	public Vec3 Translation
	{
		get
		{
			GetTranslation(EntityParent.Id, out var translation);
			return translation;
		}
		set => SetTranslation(EntityParent.Id, ref value);
	}

	public Quat Rotation
	{
		get
		{
			GetRotation(EntityParent.Id, out var rotation);
			return rotation;
		}
		set => SetRotation(EntityParent.Id, ref value);
	}

	public Vec3 Scale
	{
		get
		{
			GetScale(EntityParent.Id, out var scale);
			return scale;
		}
		set => SetScale(EntityParent.Id, ref value);
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

	public bool HasMaterial(uint p_index) { return HasMaterialInternal(EntityParent.Id, p_index); }

	public Material GetMaterial(uint p_index) { return !HasMaterial(p_index) ? null : new Material(EntityParent.Id, p_index); }
}

public class CameraComponent : Component
{
	public enum EProjectionType
	{
		Perspective,
		Orthographic
	}

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern bool GetPrimary(uint p_entity_id);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetPrimary(uint p_entity_id, ref bool p_primary);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetProjectionType(uint p_entity_id, out EProjectionType p_projection_type);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetProjectionType(uint p_entity_id, ref EProjectionType p_projection_type);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetPerspectiveFov(uint p_entity_id, out float p_perspective_fov);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetPerspectiveFov(uint p_entity_id, ref float p_perspective_fov);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetPerspectiveNear(uint p_entity_id, out float p_perspective_near);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetPerspectiveNear(uint p_entity_id, ref float p_perspective_near);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetPerspectiveFar(uint p_entity_id, out float p_perspective_far);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetPerspectiveFar(uint p_entity_id, ref float p_perspective_far);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetOrthoSize(uint p_entity_id, out float p_ortho_size);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetOrthoSize(uint p_entity_id, ref float p_ortho_size);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetOrthoNear(uint p_entity_id, out float p_ortho_near);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetOrthoNear(uint p_entity_id, ref float p_ortho_near);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetOrthoFar(uint p_entity_id, out float p_ortho_far);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetOrthoFar(uint p_entity_id, ref float p_ortho_far);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetProjectionMatrix(uint p_entity_id, out Mat4 p_projection_matrix);

	public bool Primary
	{
		get => GetPrimary(EntityParent.Id);
		set => SetPrimary(EntityParent.Id, ref value);
	}

	public EProjectionType ProjectionType
	{
		get
		{
			GetProjectionType(EntityParent.Id, out var projection_type);
			return projection_type;
		}
		set => SetProjectionType(EntityParent.Id, ref value);
	}

	public float PerspectiveFov
	{
		get
		{
			GetPerspectiveFov(EntityParent.Id, out var perspective_fov);
			return perspective_fov;
		}
		set => SetPerspectiveFov(EntityParent.Id, ref value);
	}

	public float PerspectiveNear
	{
		get
		{
			GetPerspectiveNear(EntityParent.Id, out var perspective_near);
			return perspective_near;
		}
		set => SetPerspectiveNear(EntityParent.Id, ref value);
	}

	public float PerspectiveFar
	{
		get
		{
			GetPerspectiveFar(EntityParent.Id, out var perspective_far);
			return perspective_far;
		}
		set => SetPerspectiveFar(EntityParent.Id, ref value);
	}

	public float OrthoNear
	{
		get
		{
			GetOrthoNear(EntityParent.Id, out var ortho_near);
			return ortho_near;
		}
		set => SetOrthoNear(EntityParent.Id, ref value);
	}

	public float OrthoFar
	{
		get
		{
			GetOrthoFar(EntityParent.Id, out var ortho_far);
			return ortho_far;
		}
		set => SetOrthoFar(EntityParent.Id, ref value);
	}

	public Mat4 ProjectionMatrix
	{
		get
		{
			GetProjectionMatrix(EntityParent.Id, out var projection_matrix);
			return projection_matrix;
		}
	}
}

public class DirectionalLightComponent : Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetRadiance(uint p_entity_id, out Vec3 p_out_radiance);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetRadiance(uint p_entity_id, ref Vec3 p_radiance);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetMultiplier(uint p_entity_id, out float p_out_multiplier);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetMultiplier(uint p_entity_id, ref float p_multiplier);

	public Vec3 Radiance
	{
		get
		{
			GetRadiance(EntityParent.Id, out var radiance);
			return radiance;
		}
		set => SetRadiance(EntityParent.Id, ref value);
	}

	public float Multiplier
	{
		get
		{
			GetMultiplier(EntityParent.Id, out var multiplier);
			return multiplier;
		}
		set => SetMultiplier(EntityParent.Id, ref value);
	}
}

public class PointLightComponent : Component
{
	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetRadiance(uint p_entity_id, out Vec3 p_out_radiance);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetRadiance(uint p_entity_id, ref Vec3 p_radiance);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void GetMultiplier(uint p_entity_id, out float p_out_multiplier);

	[MethodImpl(MethodImplOptions.InternalCall)]
	private static extern void SetMultiplier(uint p_entity_id, ref float p_multiplier);

	public Vec3 Radiance
	{
		get
		{
			GetRadiance(EntityParent.Id, out var radiance);
			return radiance;
		}
		set => SetRadiance(EntityParent.Id, ref value);
	}

	public float Multiplier
	{
		get
		{
			GetMultiplier(EntityParent.Id, out var multiplier);
			return multiplier;
		}
		set => SetMultiplier(EntityParent.Id, ref value);
	}
}